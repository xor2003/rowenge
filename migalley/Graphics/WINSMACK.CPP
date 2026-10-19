/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

//------------------------------------------------------------------------------
//Filename       winsmack.cpp
//System         Mig Alley
//Author         Dave Whiteside
//Date           Sun 19 Jul 1998
//Description    Smacker interface for migalley
// getfreediskspace
//------------------------------------------------------------------------------
#include "WIN32_COMPAT.H" //RERUN
#include "SDL.h"
#define DIRECTINPUT_VERSION 0x700 // RERUN
#include "dinput_stub.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <string.h>

#include	"DOSDEFS.H"
#include	"DISPLAY.H"
#include	"MYERROR.H"
#include	"SMACK.H"
#include	"FILEMAN.H"
#include	"WORLDINC.H"
#include	"MILES.H"
#include	"WINMOVE.H"
#include    "MFC_stub.h"
#include    "smacker.h"	// vendored libsmacker (engine/smacker)

// libsmacker playback state
static smk smk_video = nullptr;
static int smk_audio_track = -1;         // enabled track, -1 = none
static SDL_AudioDeviceID audio_dev = 0;
static Smack* smk_info = nullptr;
static std::vector<unsigned char> frame_buffer;	// RGB24, palette-expanded
static int frame_width = 0;
static int frame_height = 0;             // decoded rows
static int frame_draw_height = 0;        // rows after SMK y-scale
static int frame_size = 0;
static Uint32 last_frame_time = 0;

static Smack* smk_play=nullptr;
static int	Xoff=0;
static int	Yoff=0;
static int Woff=0;

/*
	UWord	DoSmack(intptr_t h)
	Returns Number of frames left [0 if finished]
	Inputs	MigWindow Handle


*/
extern	int	introsmk;

// Any-key video interrupt: arms the one-shot skip flag that DoSmack()
// consumes on its next call. Gated on smk_play so a keypress with no video
// playing can't leave a stale skip armed for the next OpenSmack().
void	InterruptSmack()
{
	if (smk_play)
		introsmk = 1;
}

#ifdef MIG_DEMO_VER
extern DPlay _DPlay;
#endif

void	CloseSmack()
{
	if (smk_video) {
		smk_close(smk_video);
		smk_video = nullptr;
	}
	smk_audio_track = -1;
	if (audio_dev) {
		SDL_CloseAudioDevice(audio_dev);
		audio_dev = 0;
	}
	if (smk_info) {
		delete smk_info;
		smk_info = nullptr;
	}
	smk_play = nullptr;
	frame_buffer.clear();
	//std::cout << "CloseSmack called" << std::endl;
}

Smack* OpenSmack(FileNum filenumber,intptr_t win, int x, int y,int w, int h)
{
	char	filepath[256];

	Xoff=x;
	Yoff=y;
	Woff=w;
	last_frame_time = 0;

	CloseSmack();
	if (!filenumber) return nullptr;

	FILEMAN.namenumberedfile(filenumber, filepath);

	smk_video = smk_open_file(filepath, SMK_MODE_DISK);
	if (!smk_video) {
		std::cerr << "OpenSmack: libsmacker failed on " << filepath << std::endl;
		return nullptr;
	}

	unsigned long fcount = 0;
	double usf = 0;
	smk_info_all(smk_video, nullptr, &fcount, &usf);

	unsigned long width = 0, height = 0;
	unsigned char ymode = SMK_FLAG_Y_NONE;
	smk_info_video(smk_video, &width, &height, &ymode);
	if (!width || !height) {
		std::cerr << "OpenSmack: no video track in " << filepath << std::endl;
		smk_close(smk_video);
		smk_video = nullptr;
		return nullptr;
	}

	smk_info = new Smack;
	memset(smk_info, 0, sizeof(Smack));
	smk_info->Width = width;
	smk_info->Height = height;
	smk_info->Frames = fcount;
	smk_info->MSPerFrame = usf > 0 ? (unsigned long)(usf / 1000.0) : 1000 / 15;
	if (!smk_info->MSPerFrame) smk_info->MSPerFrame = 1;
	smk_info->FrameNum = 0;

	smk_enable_video(smk_video, 1);

	// Enable the first audio track and open SDL audio at its native spec —
	// libsmacker returns one decoded chunk per frame, so queueing the
	// chunk as each frame is presented keeps A/V in sync.
	unsigned char trackmask = 0, schans[7] = {}, sdepth[7] = {};
	unsigned long srate[7] = {};
	smk_info_audio(smk_video, &trackmask, schans, sdepth, srate);
	for (int t = 0; t < 7; ++t) {
		if (!(trackmask & (1 << t))) continue;
		smk_enable_audio(smk_video, t, 1);
		SDL_AudioSpec want, have;
		SDL_zero(want);
		want.freq = srate[t];
		want.format = sdepth[t] == 16 ? AUDIO_S16SYS : AUDIO_U8;
		want.channels = schans[t];
		want.samples = 4096;
		audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		if (audio_dev) {
			SDL_PauseAudioDevice(audio_dev, 0);
			smk_audio_track = t;
		}
		break;
	}

	// Set up frame buffer (palette indices -> RGB24 at blit time)
	frame_width = (int)width;
	frame_height = (int)height;
	frame_draw_height = frame_height *
		(ymode == SMK_FLAG_Y_DOUBLE ? 2 : 1);   // interlaced SMKs draw 2x rows
	frame_size = frame_width * frame_height * 3;
	frame_buffer.resize(frame_size);

	smk_play = smk_info;
	if (smk_first(smk_video) == SMK_ERROR) {   // decodes frame 0 + audio
		CloseSmack();
		return nullptr;
	}

	std::cout << "OpenSmack: " << filepath << " (" << width << "x" << height
	          << " @ " << (1000.0f/smk_info->MSPerFrame) << "fps)" << std::endl;

	return smk_play;
}

UWord	DoSmack(intptr_t h)
{
	if (introsmk != 0) {
		introsmk = 0;
		fprintf(stderr, "[winsmack] bail: introsmk skip\n");
		return 0;
	}

#ifdef MIG_DEMO_VER
	if (_DPlay.PossibleLobby)
		return 1;
#endif

	if (!smk_video || !smk_play) {
		fprintf(stderr, "[winsmack] bail: no video/play state\n");
		return 0;
	}

	// Queue the current frame's decoded audio chunk (one chunk per frame,
	// so this stays in step with the video pacing below).
	if (audio_dev && smk_audio_track >= 0) {
		Uint32 queued = SDL_GetQueuedAudioSize(audio_dev);
		if (queued < 88200) { // Keep ~0.5s buffered
			unsigned long abytes = smk_get_audio_size(smk_video, smk_audio_track);
			const unsigned char* abuf = smk_get_audio(smk_video, smk_audio_track);
			if (abuf && abytes)
				SDL_QueueAudio(audio_dev, abuf, abytes);
		}
	}

	Uint32 current_time = SDL_GetTicks();
	if (last_frame_time == 0) {
		last_frame_time = current_time;
	}

	if (current_time < last_frame_time + smk_play->MSPerFrame) {
		if (smk_play->Frames > 0) return smk_play->Frames - smk_play->FrameNum;
		return 1;
	}
	last_frame_time = current_time;

	// Palette-expand the current frame into RGB24 for the blit below.
	{
		const unsigned char* vpx = smk_get_video(smk_video);
		const unsigned char* pal = smk_get_palette(smk_video);
		if (!vpx || !pal) { fprintf(stderr, "[winsmack] bail: null frame/palette\n"); CloseSmack(); return 0; }
		unsigned char* out = frame_buffer.data();
		for (int i = 0, n = frame_width * frame_height; i < n; ++i) {
			const unsigned char* c = pal + 3 * vpx[i];
			out[3*i]   = c[0];
			out[3*i+1] = c[1];
			out[3*i+2] = c[2];
		}
	}

	smk_play->FrameNum++;

	HWND hWnd = (HWND)h;
	CWnd* pWnd = CWnd::FromHandle(hWnd);
	if (!pWnd) {
		fprintf(stderr, "[winsmack] bail: FromHandle null\n");
		CloseSmack();
		return 0;
	}

	CDC* pDC = pWnd->GetDC();
	if (!pDC || !pDC->m_renderer) {
		if (pDC) pWnd->ReleaseDC(pDC);
		if (smk_play->Frames > 0) return smk_play->Frames - smk_play->FrameNum;
		return 1;
	}
	SDL_Renderer* renderer = pDC->m_renderer;

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		frame_buffer.data(), frame_width, frame_height, 24, frame_width * 3,
		0x0000FF, 0x00FF00, 0xFF0000, 0);

	if (surface) {
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (texture) {
			// dstRect is in renderer LOGICAL space (e.g. the frontend's
			// virtual 800x600), not physical pixels.
			int winW = 0, winH = 0;
			SDL_RenderGetLogicalSize(renderer, &winW, &winH);
			if (winW <= 0 || winH <= 0)
				SDL_GetRendererOutputSize(renderer, &winW, &winH);

			// To play at original size, use frame_width and frame_draw_height
			// (decode height times the SMK y-scale factor).
			// Use Xoff/Yoff for position (or center if -1).
			int dstX = Xoff;
			int dstY = Yoff;
			int dstW = frame_width;
			int dstH = frame_draw_height;

			if (Woff == -1)
			{
				dstW = winW;
				dstH = (frame_draw_height * winW) / frame_width;
				dstX = 0;
				dstY = (winH - dstH) / 2;
			}
			else
			{
				if (dstX == -1) dstX = (winW - frame_width) / 2;
				if (dstY == -1) dstY = (winH - frame_draw_height) / 2;
			}

			SDL_Rect dstRect = { dstX, dstY, dstW, dstH };
			
			// By not clearing the renderer, the background image is preserved.
			// The video frame is drawn on top of it.
			SDL_RenderCopy(renderer, texture, NULL, &dstRect);
			SDL_RenderPresent(renderer);
			
			SDL_DestroyTexture(texture);
		}
		SDL_FreeSurface(surface);
	}
	
	pWnd->ReleaseDC(pDC);

	if (smk_play->Frames > 0 && smk_play->FrameNum >= smk_play->Frames) {
		fprintf(stderr, "[winsmack] bail: frames done %u/%u\n",
		        (unsigned)smk_play->FrameNum, (unsigned)smk_play->Frames);
		CloseSmack();
		return 0;
	}
	smk_next(smk_video);          // decode the next frame for next call

	if (smk_play->Frames > 0) return smk_play->Frames - smk_play->FrameNum;
	return 1;
}
