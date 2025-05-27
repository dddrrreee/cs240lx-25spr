#!/bin/bash


# VC4ASM
echo "ASSEMBLING QASM"
out=$( vc4asm -c mandelbrotshader.c -h mandelbrotshader.h mandelbrot.qasm 2>&1 )
exit_code=$?

if [[ -n $out ]]; then
  echo "▶ ASSEMBLY FAILED WITH OUTPUT:"
  printf '%s\n' "$out"
else
  echo "RUNNING MAKE"
  make
fi