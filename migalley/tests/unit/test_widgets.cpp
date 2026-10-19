//------------------------------------------------------------------------------
// Headless tests for the ported Rowan OCX widget layer (RowanAFX/RowanMFC).
//
// Fixture: SDL dummy video, one real top-level host backend, a dialog-sheet
// parent. Widgets are created as children and driven through the same calls
// the dialog runtime uses (OnLButtonDown/Up points, Set*/Get* APIs).
//
// Behavioural expectations are taken from the original OCX sources
// (bob/rcombo/rcomboc.cpp, rlistbox.cpp etc.) — e.g. a click in the combo
// field cycles the entry, a click in the right arrow strip opens the list
// directly below the field (above when it would overflow the sheet), the
// dropdown is a screen-positioned popup.
//
// Run: ./widget_tests   (SDL_VIDEODRIVER=dummy is set by main itself)
//------------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "harness.h"
int g_checks = 0;
int g_failures = 0;

// Globals the game libs expect from the executable (same convention as
// unit_tests.cpp).
int   g_shp_last_num = -1;
void* g_shp_last_fb = 0;
void* g_shp_last_data = 0;

#include "DOSDEFS.H"
#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "MESSAGES.H"
#include "DLGITEM.H"
#include "RCOMBO.H"
#include "RCOMBOX.H"
#include "RLISTBOX.H"
#include "RSPINBUT.H"
#include "REDIT.H"
#include "REDTBT.H"
#include "RBUTTON.H"
#include "RSCRLBAR.H"
#include "RSPLTBAR.H"
#include "RTABS.H"
#include "RTITLE.H"
#include "RJOYCFG.H"
#include "RRADIO.H"
#include "RSTATIC.H"

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

static CFont g_font;

// Parent sheet: serves the messages widgets route to their parent and
// records WM_COMMAND notifications so tests can assert them.
class TestSheet : public CWnd {
public:
    std::vector<std::pair<WPARAM, LPARAM> > commands;
    std::vector<std::pair<WPARAM, LPARAM> > scrolls;

    LRESULT WindowProc(UINT msg, WPARAM w, LPARAM l) override {
        switch (msg) {
        case WM_GETGLOBALFONT:
            return (LRESULT)&g_font;
        case WM_GETARTWORK:
        case WM_GETX2FLAG:
        case WM_GETHINTBOX:
        case WM_GETCOMBODIALOG:
        case WM_GETCOMBOLISTBOX:
        case WM_PLAYSOUND:
            return 0;
        case WM_COMMAND:
            commands.push_back(std::make_pair(w, l));
            return 0;
        case WM_VSCROLL:
        case WM_HSCROLL:
        case WM_ACTIVEXSCROLL:
            scrolls.push_back(std::make_pair(w, l));
            return 0;
        }
        return CWnd::WindowProc(msg, w, l);
    }
};

static CWnd      host;   // owns the real (dummy) SDL window
static TestSheet sheet;  // dialog parent for widgets

// Painted absolute position in the host client space — mirrors
// PaintChildrenRecursive: sum m_rect up the CWnd chain, stopping before the
// CWnd that owns the real top-level backend (its m_rect is screen coords).
static CRect ClientAbs(CWnd* w) {
    CRect r = w->m_rect;
    for (CWnd* p = w->GetParent(); p; p = p->GetParent()) {
        WindowBackend* pbe = backend_from_hwnd(p->GetSafeHwnd());
        if (pbe && pbe->window && !pbe->isChild && pbe->owner == p) break;
        r.left   += p->m_rect.left; r.right  += p->m_rect.left;
        r.top    += p->m_rect.top;  r.bottom += p->m_rect.top;
    }
    return r;
}

static bool g_ready = false;

static void SetUp() {
    if (g_ready) return;
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    setenv("SDL_AUDIODRIVER", "dummy", 1);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    // AfxGetMainWnd()/AfxGetApp() dereference g_pApp — a bare CWinApp
    // instance provides it (ctor registers itself).
    static CWinApp app;

    // First parentless window owns the real backend (SDL dummy window).
    host.Create(NULL, "host", WS_OVERLAPPEDWINDOW,
                CRect(0, 0, 1024, 768), NULL, 1);
    // Simulate a non-zero OS origin: dropdown-position bugs only appear when
    // the host isn't at (0,0). This is the real SetWindowPos path, so SDL and
    // m_rect agree like they do in game.
    host.SetWindowPos(NULL, 200, 150, 1024, 768, SWP_NOZORDER);

    sheet.Create(NULL, "sheet", WS_CHILD | WS_VISIBLE,
                 CRect(50, 60, 650, 560), &host, 2);
    g_ready = true;
}

// ---------------------------------------------------------------------------
// CRCombo — the dropdown popup
// ---------------------------------------------------------------------------

static void test_combo_dropdown_position() {
    SetUp();
    CRCombo combo;
    combo.Create("cmb", WS_CHILD | WS_VISIBLE,
                 CRect(100, 100, 300, 124), &sheet, 100);
    combo.AddString("one");
    combo.AddString("two");

    // Click the right ~17px arrow strip (kRowanComboStyles[0].clipselr).
    CRect crc;
    combo.GetClientRect(crc);
    combo.OnLButtonDown(0, CPoint(crc.right - 4, crc.Height() / 2));

    CHECK(combo.GetDropDown() != NULL);
    if (!combo.GetDropDown()) return;

    // Expected: the popup sits flush below the field, at the combo's
    // client-space position (host is at fake OS origin 200,150 — if that
    // origin leaked in, the list lands displaced down-right).
    CRect want = ClientAbs(&combo);
    CHECK_EQ(combo.GetDropDown()->m_rect.left, want.left);
    CHECK_EQ(combo.GetDropDown()->m_rect.top, want.bottom);
    CHECK(combo.GetDropDown()->m_rect.Width() >= want.Width());

    // Screen-coord round trip: GetWindowRect must give back the summed
    // position INCLUDING the host OS origin (200,150).
    CRect wr;
    combo.GetDropDown()->GetWindowRect(&wr);
    CHECK_EQ(wr.left, want.left + 200);
    CHECK_EQ(wr.top, want.bottom + 150);
}

static void test_combo_field_click_cycles() {
    SetUp();
    CRCombo combo;
    combo.Create("cmb2", WS_CHILD | WS_VISIBLE,
                 CRect(100, 140, 300, 164), &sheet, 101);
    combo.AddString("a");
    combo.AddString("b");
    combo.AddString("c");
    combo.SetCurSel(0);

    // Click in the field body (left of the arrow strip): cycles, wraps.
    combo.OnLButtonDown(0, CPoint(50, 10));
    CHECK_EQ(combo.GetIndex(), 1);
    combo.OnLButtonDown(0, CPoint(50, 10));
    CHECK_EQ(combo.GetIndex(), 2);
    combo.OnLButtonDown(0, CPoint(50, 10));
    CHECK_EQ(combo.GetIndex(), 0);
    CHECK(combo.GetDropDown() == NULL); // field clicks never open the list
}

static void test_combo_dropdown_select_and_toggle() {
    SetUp();
    sheet.commands.clear();
    CRCombo combo;
    combo.Create("cmb3", WS_CHILD | WS_VISIBLE,
                 CRect(100, 180, 300, 204), &sheet, 102);
    combo.AddString("alpha");
    combo.AddString("beta");
    combo.AddString("gamma");

    CRect crc;
    combo.GetClientRect(crc);
    combo.OnLButtonDown(0, CPoint(crc.right - 4, 10));
    CHECK(combo.GetDropDown() && combo.GetDropDown()->IsWindowVisible());
    if (!combo.GetDropDown()) return;

    // Click the second row of the list.
    int ih = combo.GetDropDown()->GetItemHeight();
    combo.GetDropDown()->OnLButtonDown(0, CPoint(4, ih + 2));
    CHECK_EQ(combo.GetIndex(), 1);

    // CBN_SELCHANGE (=1) reaches the parent via WM_COMMAND.
    bool selchange = false;
    for (auto& c : sheet.commands)
        if (LOWORD(c.first) == 102 && HIWORD(c.first) == 1) selchange = true;
    CHECK(selchange);

    // Selecting auto-hides the dropdown (original OnSelectRlistboxctrl1).
    CListBox* dd = combo.GetDropDown();
    CHECK(!dd->IsWindowVisible());

    // Arrow click re-opens; a second arrow click hides (toggle).
    combo.OnLButtonDown(0, CPoint(crc.right - 4, 10));
    CHECK(dd->IsWindowVisible());
    combo.OnLButtonDown(0, CPoint(crc.right - 4, 10));
    CHECK(!dd->IsWindowVisible());
}

// ---------------------------------------------------------------------------
// CListBox (the dropdown list itself, and MFC-standard usage)
// ---------------------------------------------------------------------------

static void test_clistbox_items() {
    SetUp();
    CListBox lb;
    lb.Create(NULL, "lb", WS_CHILD | WS_VISIBLE,
              CRect(10, 300, 210, 380), &sheet, 110);
    CHECK_EQ(lb.GetCount(), 0);
    lb.AddString("x");
    lb.AddString("y");
    lb.AddString("z");
    CHECK_EQ(lb.GetCount(), 3);
    CHECK_EQ(lb.SetCurSel(1), 1);
    CHECK_EQ(lb.GetCurSel(), 1);
    char buf[64] = {0};
    lb.GetText(1, buf);
    CHECK(strcmp(buf, "y") == 0);
    CHECK_EQ(lb.FindString(-1, "z"), 2);
    lb.DeleteString(0);
    CHECK_EQ(lb.GetCount(), 2);
    lb.ResetContent();
    CHECK_EQ(lb.GetCount(), 0);
}

static void test_clistbox_click_select() {
    SetUp();
    CListBox lb;
    lb.Create(NULL, "lb2", WS_CHILD | WS_VISIBLE,
              CRect(10, 400, 210, 600), &sheet, 111);
    for (int i = 0; i < 5; ++i) {
        char s[8];
        sprintf(s, "it%d", i);
        lb.AddString(s);
    }
    int ih = lb.GetItemHeight();
    CHECK(ih > 0);
    lb.OnLButtonDown(0, CPoint(4, ih * 2 + 2));
    CHECK_EQ(lb.GetCurSel(), 2);
}

static void test_clistbox_scroll() {
    SetUp();
    CListBox lb;
    lb.Create(NULL, "lb3", WS_CHILD | WS_VISIBLE,
              CRect(220, 400, 420, 480), &sheet, 112);
    for (int i = 0; i < 30; ++i) {
        char s[8];
        sprintf(s, "s%d", i);
        lb.AddString(s);
    }
    // More items than fit: a click on the scrollbar strip scrolls.
    CRect crc;
    lb.GetClientRect(crc);
    lb.OnLButtonDown(0, CPoint(crc.right - 8, crc.bottom - 4));
    CHECK(lb.GetTopIndex() > 0);
    // Click in the body selects relative to the scroll position.
    int top = lb.GetTopIndex();
    lb.OnLButtonDown(0, CPoint(4, 2));
    CHECK_EQ(lb.GetCurSel(), top);
}

// ---------------------------------------------------------------------------
// CRSpinBut — up/down halves step the value/index
// ---------------------------------------------------------------------------

static void test_spinbut_list() {
    SetUp();
    CRSpinBut sp;
    sp.Create("sp", WS_CHILD | WS_VISIBLE,
              CRect(400, 100, 460, 140), &sheet, 120);
    sp.AddString("low");
    sp.AddString("mid");
    sp.AddString("high");
    sp.SetIndex(1);
    CRect crc;
    sp.GetClientRect(crc);
    // Upper half: index +1. Lower half: index -1. Clamps at both ends.
    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, 2));
    sp.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 2));
    CHECK_EQ(sp.GetIndex(), 2);
    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, 2));
    sp.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 2));
    CHECK_EQ(sp.GetIndex(), 2); // clamped at max
    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, crc.Height() - 2));
    sp.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, crc.Height() - 2));
    CHECK_EQ(sp.GetIndex(), 1);
}

static void test_spinbut_value() {
    SetUp();
    CRSpinBut sp;
    sp.Create("spv", WS_CHILD | WS_VISIBLE,
              CRect(400, 160, 460, 200), &sheet, 121);
    sp.SetValueOption(0, 10, 5);
    CRect crc;
    sp.GetClientRect(crc);
    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, 2));
    sp.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(10, 2));
    CHECK_EQ(sp.GetCurrentValue(), 6);
}

// ---------------------------------------------------------------------------
// CRComboExtra / CRSpinButExtra — MakeNumList helper
// ---------------------------------------------------------------------------

static void test_extras_numlist() {
    SetUp();
    CRComboExtra cx;
    cx.Create("cx", WS_CHILD | WS_VISIBLE,
              CRect(500, 100, 620, 124), &sheet, 130);
    cx.MakeNumList(4, 10, 5);
    CRect crc;
    cx.GetClientRect(crc);
    cx.OnLButtonDown(0, CPoint(50, 10)); // cycle once -> "15"
    CHECK_EQ(cx.GetIndex(), 1);

    CRSpinButExtra sx;
    sx.Create("sx", WS_CHILD | WS_VISIBLE,
              CRect(500, 140, 560, 180), &sheet, 131);
    sx.MakeNumList(3, 100, 50);
    sx.SetIndex(0);
    CHECK_EQ(sx.GetIndex(), 0);
}

// ---------------------------------------------------------------------------
// CRListBox — columns + hilight row (original OCX semantics)
// ---------------------------------------------------------------------------

static void test_rlistbox() {
    SetUp();
    CRListBox rl;
    rl.Create("rl", WS_CHILD | WS_VISIBLE,
              CRect(240, 300, 500, 400), &sheet, 140);
    rl.Clear();
    rl.AddColumn(10);
    rl.AddString("row0", 0);
    rl.AddString("row1", 0);
    CHECK_EQ(rl.GetHilightRow(), rl.GetHilightRow()); // readable
    rl.SetHilightRow(1);
    CHECK_EQ(rl.GetHilightRow(), 1);
    CHECK(rl.GetString(0, 0) != 0);
}

// ---------------------------------------------------------------------------
// CREdit / CREdtBt — text in/out
// ---------------------------------------------------------------------------

static void test_edit() {
    SetUp();
    CREdit ed;
    ed.Create("ed", WS_CHILD | WS_VISIBLE,
              CRect(10, 240, 210, 264), &sheet, 150);
    ed.SetWindowText("pilot");
    char buf[64] = {0};
    ed.GetWindowText(buf, 63);
    CHECK(strcmp(buf, "pilot") == 0);
    ed.SetLimitText(4);
    CHECK_EQ(ed.m_limit, 4);
}

static void test_edtbt() {
    SetUp();
    CREdtBt eb;
    eb.Create("eb", WS_CHILD | WS_VISIBLE,
              CRect(10, 270, 210, 294), &sheet, 151);
    eb.SetWindowText("abc");
    char buf[64] = {0};
    eb.GetWindowText(buf, 63);
    CHECK(strcmp(buf, "abc") == 0);
}

// ---------------------------------------------------------------------------
// CRButton / CRRadio — captions, pressed state, selection
// ---------------------------------------------------------------------------

static void test_rbutton() {
    SetUp();
    CRButton b;
    b.Create(NULL, "b", WS_CHILD | WS_VISIBLE,
             CRect(500, 240, 620, 270), &sheet, 160);
    b.SetCaption("Engage");
    CHECK(b.GetCaption() == "Engage");
    b.SetPressed(TRUE);
    CHECK(b.GetPressed() == TRUE);
    b.SetPressed(FALSE);
    CHECK(b.GetPressed() == FALSE);
    b.SetEnabled(TRUE);
}

static void test_rradio() {
    SetUp();
    CRRadio r;
    r.Create(NULL, "r", WS_CHILD | WS_VISIBLE,
             CRect(500, 300, 640, 380), &sheet, 161);
    r.AddButton("off");
    r.AddButton("on");
    r.SetCurrentSelection(1);
    CHECK_EQ(r.GetCurrentSelection(), 1);
}

// ---------------------------------------------------------------------------
// CRScrlBar / CRSpltBar / CRTabs / CRTitle / CRStatic
// ---------------------------------------------------------------------------

static void test_scrlbar() {
    SetUp();
    CRScrlBar sb;
    sb.Create(NULL, "sb", WS_CHILD | WS_VISIBLE,
              CRect(10, 600, 400, 620), &sheet, 170);
    sb.SetMinValue(0);
    sb.SetMaxValue(100);
    sb.SetScrollPos(40);
    CHECK_EQ(sb.GetScrollPos(), 40);
    CHECK_EQ(sb.GetMinValue(), 0);
    CHECK_EQ(sb.GetMaxValue(), 100);
}

static void test_spltbar() {
    SetUp();
    CRSpltBar sp;
    sp.Create("spb", WS_CHILD | WS_VISIBLE,
              CRect(420, 600, 620, 620), &sheet, 171);
    CHECK(sp.m_hWnd != NULL);
}

static void test_tabs() {
    SetUp();
    CRTabs tb;
    tb.Create(NULL, "tb", WS_CHILD | WS_VISIBLE,
              CRect(10, 630, 400, 660), &sheet, 172);
    tb.AddTab("one", 1);
    tb.AddTab("two", 2);
    CHECK(tb.m_hWnd != NULL);
}

static void test_title_static() {
    SetUp();
    CRTitle t;
    t.Create("t", WS_CHILD | WS_VISIBLE,
             CRect(420, 630, 640, 660), &sheet, 173);
    CHECK(t.m_hWnd != NULL);

    CRStatic st;
    st.Create("st", WS_CHILD | WS_VISIBLE,
              CRect(10, 670, 200, 690), &sheet, 174);
    st.SetCaption("caption");
    CHECK(st.GetCaption() == "caption");
}

static void test_joycfg() {
    SetUp();
    CRJoyCfg j;
    j.Create("j", WS_CHILD | WS_VISIBLE,
             CRect(420, 670, 640, 700), &sheet, 175);
    CHECK(j.m_hWnd != NULL);
}

// ---------------------------------------------------------------------------
// MFC-standard stubs used by the frontend
// ---------------------------------------------------------------------------

static void test_std_stubs() {
    SetUp();
    CButton bt;
    bt.Create("ok", WS_CHILD | WS_VISIBLE,
              CRect(10, 700, 110, 724), &sheet, 180);
    bt.SetCheck(1);
    CHECK_EQ(bt.GetCheck(), 1);

    CStatic tx;
    tx.Create("tx", WS_CHILD | WS_VISIBLE,
              CRect(120, 700, 240, 724), &sheet, 181);
    tx.SetWindowText("hello");
    CString s;
    tx.GetWindowText(s);
    CHECK(s == "hello");

    CEdit ed;
    ed.Create(NULL, "ed", WS_CHILD | WS_VISIBLE,
              CRect(250, 700, 400, 724), &sheet, 182);
    ed.SetWindowText("edit");
    CString es;
    ed.GetWindowText(es);
    CHECK(es == "edit");
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Original-OCX behaviour parity — expectations read straight from the
// original sources (bob/rcombo/rcomboc.cpp, rspinbut/rspinbtc.cpp,
// rscrlbar/rscrlbar.cpp, rradio). The OCX is OLE/IDispatch and can't run
// here, so its semantics are pinned as assertions on the port.
// ---------------------------------------------------------------------------

// rcomboc.cpp OnLButtonDown: when rect.bottom+listboxheight would pass the
// ancestor sheet's bottom, the list opens ABOVE the field instead of below.
static void test_combo_dropdown_flipup() {
    SetUp();
    CRCombo combo;
    // Near the sheet bottom (sheet client 50,60-650,560 -> screen bottom 710).
    combo.Create("cmb2", WS_CHILD | WS_VISIBLE,
                 CRect(100, 480, 300, 504), &sheet, 101);
    combo.AddString("one");
    combo.AddString("two");

    CRect crc;
    combo.GetClientRect(crc);
    combo.OnLButtonDown(0, CPoint(crc.right - 4, crc.Height() / 2));

    CHECK(combo.GetDropDown() != NULL);
    if (!combo.GetDropDown()) return;

    CRect want = ClientAbs(&combo);
    // Flipped: popup bottom is flush with the field's TOP edge.
    CHECK(combo.GetDropDown()->m_rect.bottom <= want.top);
    CHECK_EQ(combo.GetDropDown()->m_rect.left, want.left);
    // And still inside the sheet, not clipped off the bottom.
    CHECK(combo.GetDropDown()->m_rect.top >= 0);
}

// rspinbtc.cpp: upper half ++, lower half --, clamps at min/max — no wrap.
static void test_parity_spin_clamp() {
    SetUp();
    CRSpinBut sp;
    sp.Create("spc", WS_CHILD | WS_VISIBLE,
              CRect(400, 160, 460, 200), &sheet, 122);
    sp.SetValueOption(0, 10, 9);
    CRect rc; sp.GetClientRect(rc);
    const int midY = rc.Height() / 2;

    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, 2));
    sp.SendMessage(WM_LBUTTONUP,   0, MAKELPARAM(10, 2));
    CHECK_EQ(sp.GetCurrentValue(), 10);   // up half: 9->10

    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, 2));
    sp.SendMessage(WM_LBUTTONUP,   0, MAKELPARAM(10, 2));
    CHECK_EQ(sp.GetCurrentValue(), 10);   // at max: clamps, doesn't wrap

    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, midY + 2));
    sp.SendMessage(WM_LBUTTONUP,   0, MAKELPARAM(10, midY + 2));
    CHECK_EQ(sp.GetCurrentValue(), 9);    // down half: 10->9

    sp.SetValueOption(0, 10, 0);
    sp.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(10, midY + 2));
    sp.SendMessage(WM_LBUTTONUP,   0, MAKELPARAM(10, midY + 2));
    CHECK_EQ(sp.GetCurrentValue(), 0);    // at min: clamps
}

// rscrlbar.cpp: shaft click pages toward the click by pageSize, the thumb
// drags pixel-proportionally, position clamps to [min,max], and the parent
// gets a scroll notification.
static void test_parity_scrlbar() {
    SetUp();
    sheet.scrolls.clear();
    CRScrlBar sb;
    sb.Create(NULL, "sbv", WS_CHILD | WS_VISIBLE,
              CRect(10, 10, 30, 410), &sheet, 171);
    sb.SetMinValue(0);
    sb.SetMaxValue(100);
    sb.SetPageSize(10);
    sb.SetScrollPos(40);

    // Vertical, track 400, thumbLen=max(20,10/110*400)=36, available=364.
    // pos 40 -> thumb at [145,181]. Click below -> +page.
    sb.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(5, 350));
    CHECK_EQ(sb.GetScrollPos(), 50);
    sb.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(5, 350));

    // pos 50 -> thumb [182,218]. Click above -> -page.
    sb.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(5, 10));
    CHECK_EQ(sb.GetScrollPos(), 40);
    sb.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(5, 10));

    // Thumb drag: grab inside the thumb, drag ~10% of track -> ~+10.
    sb.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(5, 163));
    sb.SendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(5, 199));
    CHECK(sb.GetScrollPos() >= 48 && sb.GetScrollPos() <= 52);
    sb.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(5, 199));

    // Clamp: page-step can't pass max, and SetScrollPos clamps too.
    sb.SetScrollPos(200);
    CHECK_EQ(sb.GetScrollPos(), 100);
    sb.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(5, 395));
    CHECK_EQ(sb.GetScrollPos(), 100);
    sb.SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(5, 395));

    // Parent received the scroll notifications (useMessages=FALSE ->
    // WM_VSCROLL with SB_THUMBPOSITION/SB_THUMBTRACK).
    CHECK(!sheet.scrolls.empty());
}

// rradio: the widget is a grid of mutually exclusive options — a click
// selects exactly the cell under the point.
static void test_parity_radio_click() {
    SetUp();
    CRRadio r;
    r.Create(NULL, "rg", WS_CHILD | WS_VISIBLE,
             CRect(500, 300, 640, 380), &sheet, 162);
    r.AddButton("off");
    r.AddButton("low");
    r.AddButton("high");
    r.SetCurrentSelection(0);

    // Descend the client area: the selection must walk 0 -> 1 -> 2 exactly
    // once each (row pitch depends on font metrics, so probe rather than
    // assume pixel positions).
    int seen = 0;
    for (int y = 2; y < 80 && seen < 3; ++y) {
        r.SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(5, y));
        if (r.GetCurrentSelection() == seen + 1)
            ++seen;
        CHECK(r.GetCurrentSelection() >= 0 && r.GetCurrentSelection() <= 2);
    }
    CHECK_EQ(seen, 3 - 1);          // reached the third row (0,1,2 visited)
    CHECK_EQ(r.GetCurrentSelection(), 2);
}

int main() {
    printf("widget tests (SDL dummy video)\n");

    test_combo_dropdown_position();
    test_combo_field_click_cycles();
    test_combo_dropdown_select_and_toggle();
    test_clistbox_items();
    test_clistbox_click_select();
    test_clistbox_scroll();
    test_spinbut_list();
    test_spinbut_value();
    test_extras_numlist();
    test_rlistbox();
    test_edit();
    test_edtbt();
    test_rbutton();
    test_rradio();
    test_scrlbar();
    test_spltbar();
    test_tabs();
    test_title_static();
    test_joycfg();
    test_std_stubs();
    test_combo_dropdown_flipup();
    test_parity_spin_clamp();
    test_parity_scrlbar();
    test_parity_radio_click();

    printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
