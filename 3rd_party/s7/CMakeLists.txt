cmake_minimum_required (VERSION 3.0)
project (s7)

add_library (s7 STATIC
  s7.c
  s7.h)

set (s7_FILES
  cload.scm
  libc.scm
  libdl.scm
  libgdbm.scm
  libgsl.scm
  libm.scm
  libutf8proc.scm
  lint.scm
  mockery.scm
  profile.scm
  r7rs.scm
  repl.scm
  s7test.scm
  snd-lint.scm
  stuff.scm
  write.scm
  )

set_property(TARGET s7 PROPERTY FOLDER "3rd/s7")

