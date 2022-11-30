if empty(glob('~/.vim/autoload/plug.vim'))
  silent !curl -fLo ~/.vim/autoload/plug.vim --create-dirs
    \ https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  autocmd VimEnter * PlugInstall --sync | source $MYVIMRC
endif

call plug#begin('~/.vim/plugged')
	Plug 'https://github.com/junegunn/vim-github-dashboard.git'
	Plug 'preservim/nerdtree', { 'on':  'NERDTreeToggle' }
	Plug 'junegunn/fzf', { 'dir': '~/.fzf', 'do': './install --all' }
	Plug 'junegunn/fzf.vim'
	Plug 'vim-airline/vim-airline'
	Plug 'vim-airline/vim-airline-themes'
	Plug 'skywind3000/asyncrun.vim'
	Plug 'tpope/vim-fugitive'
	Plug 'airblade/vim-gitgutter'
	"Plug 'vim-syntastic/syntastic'
	Plug 'rking/ag.vim'
	Plug 'ggreer/the_silver_searcher'
	Plug 'neoclide/coc.nvim', {'branch': 'release'}
	Plug 'nvie/vim-flake8'
	"Plug 'majutsushi/tagbar'
	"Plug 'ycm-core/YouCompleteMe'
	"Plug 'vim-scripts/Conque-GDB'
	"Plug 'janko/vim-test'
call plug#end()
"let g:ycm_clangd_binary_path = "/usr/bin/clangd"
set number

set rtp+=~/.fzf
set rtp+=~/.tagbar
set tags=tags

let g:gitgutter_git_executable = '/usr/bin/git'
set updatetime=100

set number relativenumber
set nu rnu
set so=999
set tabstop=4
set sw=2
"set sw=4
set foldmethod=indent
set foldignore=^
set nowrap
set diffopt+=vertical
set diffopt+=iwhite
highlight Folded ctermbg=NONE
set foldignore=^

set signcolumn=yes
hi clear SignColumn

let mapleader=" "
noremap <Leader>g :Git<CR>
noremap <Leader>x <C-v>ec
noremap <Leader>1 :!<CR>
noremap <Leader>h :split %:r.hpp<CR>
noremap <Leader>c :split %:r.cpp<CR>
noremap <Leader>s :vsplit<CR>:Files<CR>
noremap <Leader>e :Files<CR>
noremap <Leader>l :tabnew<CR>:Files<CR>
noremap <Leader>f :let @f=@%<CR>
noremap <Up> <Nop>
noremap <Down> <Nop>
noremap <Left> <Nop>
noremap <Right> <Nop>

nmap <Leader>dd :call vimspector#Launch()<CR>
nmap <Leader>dx :call vimspector#Reset()<CR>
nmap <Leader>df :call vimspector#ClearBreakpoints()<CR>
nmap <Leader>dr <Plug>VimspectorRestart
nmap <Leader>dp <Plug>VimspectorPause
nmap <Leader>db <Plug>VimspectorToggleBreakpoint
nmap <Leader>de :VimspectorEval
nmap <Leader>dw :VimspectorWatch
nmap <Leader>ds <Plug>VimspectorBalloonEval
xmap <Leader>ds <Plug>VimspectorBalloonEval
nmap <Leader>do <Plug>VimspectorStepOver
nmap <Leader>di <Plug>VimspectorStepInto
nmap <Leader>du <Plug>VimspectorStepOut
nmap <Leader>dg <Plug>VimspectorRunToCursor
nmap <Leader>dc <Plug>VimspectorContinue
nmap <Leader>dh :VimspectorShowOutput stderr<CR>

nmap <C-b> <Plug>VimspectorToggleBreakpoint
nmap <C-j> <Plug>VimspectorStepOver
nmap <C-k> <Plug>VimspectorStepInto

"func! s:CustomiseUI()
"  call win_gotoid( g:vimspector_session_windows.code )
"  nunmenu WinBar
"  call win_gotoid( g:vimspector_session_windows.variables )
"  nunmenu WinBar
"  call win_gotoid( g:vimspector_session_windows.stack_trace )
"  nunmenu WinBar
"  call win_gotoid( g:vimspector_session_windows.output )
"  nunmenu WinBar
"  call win_gotoid( g:vimspector_session_windows.tabpage )
"  nunmenu WinBar
"  call win_gotoid( g:vimspector_session_windows.watches )
"  nunmenu WinBar
"  "" Cretae our own WinBar
"  "nnoremenu WinBar.Kill :call vimspector#Stop( { 'interactive': v:true } )<CR>
"  "nnoremenu WinBar.Continue :call vimspector#Continue()<CR>
"  "nnoremenu WinBar.Pause :call vimspector#Pause()<CR>
"  "nnoremenu WinBar.Step\ Over  :call vimspector#StepOver()<CR>
"  "nnoremenu WinBar.Step\ In :call vimspector#StepInto()<CR>
"  "nnoremenu WinBar.Step\ Out :call vimspector#StepOut()<CR>
"  "nnoremenu WinBar.Restart :call vimspector#Restart()<CR>
"  "nnoremenu WinBar.Exit :call vimspector#Reset()<CR>
"endfunction
"
"augroup MyVimspectorUICustomistaion
"  autocmd!
"  autocmd User VimspectorUICreated call s:CustomiseUI()
"augroup END

noremap <S-d> :so %<CR>
noremap <S-s> :w<CR>
noremap \ <C-w>

"set statusline+=%#warningmsg#
"set statusline+=%{SyntasticStatuslineFlag()}
"set statusline+=%*
"
"let g:syntastic_always_populate_loc_list = 1
"let g:syntastic_auto_loc_list = 1
"let g:syntastic_check_on_open = 1
"let g:syntastic_check_on_wq = 0
"let g:syntastic_cpp_check_header = 1

let g:termdebug_wide=1

let g:airline#extensions#tabline#enabled = 1
set noshowmode
if !exists('g:airline_symbols')
let g:airline_symbols = {}
endif
" unicode symbols
let g:airline_left_sep = '»'
let g:airline_left_sep = '▶'
let g:airline_right_sep = '«'
let g:airline_right_sep = '◀'
let g:airline_symbols.linenr = '☰'
let g:airline_symbols.paste = 'ρ'
let g:airline_symbols.paste = 'Þ'
let g:airline_symbols.notexists = '∄'
let g:airline_symbols.whitespace = 'Ξ'

" powerline symbols
let g:airline_left_sep = ''
let g:airline_left_alt_sep = ''
let g:airline_right_sep = ''
let g:airline_right_alt_sep = ''
let g:airline_symbols.branch = ''
let g:airline_symbols.readonly = ''
let g:airline_symbols.linenr = '☰'
let g:airline_symbols.maxlinenr = ''
let g:airline#extensions#tabline#left_sep = ''
let g:airline#extensions#tabline#left_alt_sep = ''

":autocmd BufWritePost * silent !%
augroup quickfix
    autocmd!
    autocmd FileType qf setlocal wrap
augroup END
set exrc
highlight Folded ctermbg=NONE
" TextEdit might fail if hidden is not set.
set hidden

" Some servers have issues with backup files, see #649.
set nobackup
set nowritebackup

" Give more space for displaying messages.
set cmdheight=2

" Having longer updatetime (default is 4000 ms = 4 s) leads to noticeable
" delays and poor user experience.
set updatetime=300

" Don't pass messages to |ins-completion-menu|.
set shortmess+=c

" NOTE: Use command ':verbose imap <tab>' to make sure tab is not mapped by
" other plugin before putting this into your config.
inoremap <silent><expr> <TAB>
      \ pumvisible() ? "\<C-n>" :
      \ <SID>check_back_space() ? "\<TAB>" :
      \ coc#refresh()
inoremap <expr><S-TAB> pumvisible() ? "\<C-p>" : "\<C-h>"

function! s:check_back_space() abort
  let col = col('.') - 1
  return !col || getline('.')[col - 1]  =~# '\s'
endfunction

" Use <c-space> to trigger completion.
if has('nvim')
  inoremap <silent><expr> <c-space> coc#refresh()
else
  inoremap <silent><expr> <c-@> coc#refresh()
endif

" Make <CR> auto-select the first completion item and notify coc.nvim to
" format on enter, <cr> could be remapped by other vim plugin
inoremap <silent><expr> <cr> pumvisible() ? coc#_select_confirm()
                              \: "\<C-g>u\<CR>\<c-r>=coc#on_enter()\<CR>"

" Use `[g` and `]g` to navigate diagnostics
" Use `:CocDiagnostics` to get all diagnostics of current buffer in location list.
nmap <silent> [g <Plug>(coc-diagnostic-prev)
nmap <silent> ]g <Plug>(coc-diagnostic-next)

" GoTo code navigation.
nmap <silent> gd <Plug>(coc-definition)
nmap <silent> gy <Plug>(coc-type-definition)
nmap <silent> gi <Plug>(coc-implementation)
nmap <silent> gr <Plug>(coc-references)

" Use K to show documentation in preview window.
nnoremap <silent> K :call <SID>show_documentation()<CR>

function! s:show_documentation()
  if (index(['vim','help'], &filetype) >= 0)
    execute 'h '.expand('<cword>')
  elseif (coc#rpc#ready())
    call CocActionAsync('doHover')
  else
    execute '!' . &keywordprg . " " . expand('<cword>')
  endif
endfunction

" Highlight the symbol and its references when holding the cursor.
autocmd CursorHold * silent call CocActionAsync('highlight')

" Symbol renaming.
nmap <leader>rn <Plug>(coc-rename)

"" Formatting selected code.
"xmap <leader>f  <Plug>(coc-format-selected)
"nmap <leader>f  <Plug>(coc-format-selected)

augroup mygroup
  autocmd!
  " Setup formatexpr specified filetype(s).
  autocmd FileType typescript,json setl formatexpr=CocAction('formatSelected')
  " Update signature help on jump placeholder.
  autocmd User CocJumpPlaceholder call CocActionAsync('showSignatureHelp')
augroup end

" Applying codeAction to the selected region.
" Example: `<leader>aap` for current paragraph
xmap <leader>a  <Plug>(coc-codeaction-selected)
nmap <leader>a  <Plug>(coc-codeaction-selected)

" Remap keys for applying codeAction to the current buffer.
nmap <leader>ac  <Plug>(coc-codeaction)
" Apply AutoFix to problem on the current line.
nmap <leader>qf  <Plug>(coc-fix-current)

" Map function and class text objects
" NOTE: Requires 'textDocument.documentSymbol' support from the language server.
xmap if <Plug>(coc-funcobj-i)
omap if <Plug>(coc-funcobj-i)
xmap af <Plug>(coc-funcobj-a)
omap af <Plug>(coc-funcobj-a)
xmap ic <Plug>(coc-classobj-i)
omap ic <Plug>(coc-classobj-i)
xmap ac <Plug>(coc-classobj-a)
omap ac <Plug>(coc-classobj-a)

"" Remap <C-f> and <C-b> for scroll float windows/popups.
"if has('nvim-0.4.0') || has('patch-8.2.0750')
"  nnoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? coc#float#scroll(1) : "\<C-f>"
"  nnoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? coc#float#scroll(0) : "\<C-b>"
"  inoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? "\<c-r>=coc#float#scroll(1)\<cr>" : "\<Right>"
"  inoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? "\<c-r>=coc#float#scroll(0)\<cr>" : "\<Left>"
"  vnoremap <silent><nowait><expr> <C-f> coc#float#has_scroll() ? coc#float#scroll(1) : "\<C-f>"
"  vnoremap <silent><nowait><expr> <C-b> coc#float#has_scroll() ? coc#float#scroll(0) : "\<C-b>"
"endif

" Use CTRL-S for selections ranges.
" Requires 'textDocument/selectionRange' support of language server.
nmap <silent> <C-s> <Plug>(coc-range-select)
xmap <silent> <C-s> <Plug>(coc-range-select)

" Add `:Format` command to format current buffer.
command! -nargs=0 Format :call CocAction('format')

" Add `:Fold` command to fold current buffer.
command! -nargs=? Fold :call     CocAction('fold', <f-args>)

" Add `:OR` command for organize imports of the current buffer.
command! -nargs=0 OR   :call     CocAction('runCommand', 'editor.action.organizeImport')

" Add (Neo)Vim's native statusline support.
" NOTE: Please see `:h coc-status` for integrations with external plugins that
" provide custom statusline: lightline.vim, vim-airline.
set statusline^=%{coc#status()}%{get(b:,'coc_current_function','')}

"" Mappings for CoCList
"" Show all diagnostics.
"nnoremap <silent><nowait> <space>a  :<C-u>CocList diagnostics<cr>
"" Manage extensions.
"nnoremap <silent><nowait> <space>e  :<C-u>CocList extensions<cr>
"" Show commands.
"nnoremap <silent><nowait> <space>c  :<C-u>CocList commands<cr>
"" Find symbol of current document.
"nnoremap <silent><nowait> <space>o  :<C-u>CocList outline<cr>
"" Search workspace symbols.
"nnoremap <silent><nowait> <space>s  :<C-u>CocList -I symbols<cr>
"" Do default action for next item.
"nnoremap <silent><nowait> <space>j  :<C-u>CocNext<CR>
"" Do default action for previous item.
"nnoremap <silent><nowait> <space>k  :<C-u>CocPrev<CR>
"" Resume latest coc list.
"nnoremap <silent><nowait> <space>p  :<C-u>CocListResume<CR>
colorscheme elflord
hi! SignColumn ctermbg=NONE guibg=NONE
hi! Folded ctermbg=NONE guibg=NONE
