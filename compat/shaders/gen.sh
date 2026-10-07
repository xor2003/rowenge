#!/bin/sh
# Regenerate d3dvk_spv.h from the GLSL sources.  Needs glslangValidator + xxd.
# Three modules: vertex, fragment, and the -DWBUF fragment variant (the
# w-buffer depth map is compile-time — a conditional gl_FragDepth store is
# not reliable across drivers; see d3dvk.frag).
set -e
cd "$(dirname "$0")"
GLSLANG="${GLSLANG:-glslangValidator}"
"$GLSLANG" -V --target-env vulkan1.0 d3dvk.vert -o d3dvk.vert.spv
"$GLSLANG" -V --target-env vulkan1.0 d3dvk.frag -o d3dvk.frag.spv
"$GLSLANG" -V --target-env vulkan1.0 -DWBUF d3dvk.frag -o d3dvk.fragw.spv
{
  echo '// Generated from shaders/d3dvk.{vert,frag} via glslangValidator -V'
  echo '// --target-env vulkan1.0 (+ -DWBUF for fragw) + xxd -i.'
  echo '// Regenerate with compat/shaders/gen.sh.'
  echo '#ifndef ROWAN_D3DVK_SPV_H'
  echo '#define ROWAN_D3DVK_SPV_H'
  xxd -i d3dvk.vert.spv
  xxd -i d3dvk.frag.spv
  xxd -i d3dvk.fragw.spv
  echo '#endif'
} | sed 's/unsigned char /static const unsigned char /;s/unsigned int /static const unsigned int /' > d3dvk_spv.h
