######################################################################
## bbpager.bb: Style file for bbpager when using Blackbox           ##
######################################################################

 #############################################################
 ## By default values are taken from  the Blackbox style.   ##
 ## If you want to override something just uncomment it     ##
 ## and change the value                                    ##
 #############################################################

 #############################################################
 ## the default Blackbox style mappings are:
 ##  bbpager.frame -> toolbar
 ##  bbpager.frame.borderColor -> toolbar.label.TextColor
 ##  bbpager.desktop.focus -> toolbar.windowlabel
 ##  bbpager.desktop.focus.borderColor -> toolbar.windowlabel.TextColor
 ##  bbpager.desktop -> toolbar.label
 ##  bbpager.window.focus -> window.label.focus
 ##  bbpager.window.focus.borderColor -> window.label.focus.TextColor
 ##  bbpager.window -> window.label.unfocus
 ##  bbpager.window.borderColor -> window.label.unfocus.TextColor
 #############################################################

!## define frame style ##
!bbpager.frame:                 Sunken Gradient Vertical Bevel1
!bbpager.frame.color:           slategrey
!bbpager.frame.colorTo:         darkslategrey
!bbpager.frame.borderColor:     lightgrey
!bbpager.frame.borderWidth:           1

!## define desktop ##
!bbpager.desktop:               Sunken Gradient Horizontal Bevel1
!bbpager.desktop.color:         slategrey
!bbpager.desktop.colorTo:       darkslategrey

!## set the focused desktop style to none, border or texture ##
!## also border2, border3 for slightly different borders ##
! bbpager.desktop.focusStyle:     border

!## define focused desktop ##
!bbpager.desktop.focus:         Raised Gradient Vertical Bevel1
!bbpager.desktop.focus.color:   darkslategrey
!bbpager.desktop.focus.colorTo: slategrey
!## also defines desktop numbers colour ##
!bbpager.desktop.focus.borderColor:    lightgrey

!## define window ##
!bbpager.window:                Sunken Gradient Vertical Bevel2
!bbpager.window.color:          slategrey
!bbpager.window.colorTo:        darkslategrey
!bbpager.window.borderColor:   black

!## set the window focus style to none, border or texture ##
! bbpager.window.focusStyle:		texture

!## define window focus ##
!bbpager.window.focus:          Raised Gradient Vertical Bevel2
!bbpager.window.focus.color:    rgb:c/9/6
!bbpager.window.focus.colorTo:  rgb:8/6/4
!bbpager.window.focus.borderColor:     lightgrey

!## set bevelwidth
!bbpager.bevelWidth:            4
