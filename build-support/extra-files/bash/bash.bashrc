[[ $- != *i* ]] && return

HISTCONTROL=ignoredups
HISTSIZE=-1
HISTFILESIZE=-1

PS1='\[\e[32m\]\u@\h:\[\e[34m\]\w\[\e[m\]\$ '

alias ls='ls --color=auto'