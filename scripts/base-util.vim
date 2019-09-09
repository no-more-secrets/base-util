set sw=2
set shiftwidth=2
set tabstop=2

" Automatically format the C++ source files just before saving.
"autocmd BufWritePre *.hpp,*.cpp call ClangFormatAll()

" Get the folder in which this file resides.
let s:path = expand( '<sfile>:p:h' )

" We set this ycm global variable to point YCM to the conf script.  The
" reason we don't just put a .ycm_extra_conf.py in the root folder
" (which YCM would then find on its own without our help) is that we
" want to keep the folder structure organized with all scripts in the
" scripts folder.
let g:ycm_global_ycm_extra_conf = s:path . '/scripts/ycm_extra_conf.py'
