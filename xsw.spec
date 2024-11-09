Summary: Space oriented network game system.
Name: xsw
Version: 1.33h C++ Edition
Release: 1
URL: http://fox.mit.edu/xsw/
Source: ftp://ftp.mit.edu/pub/xsw/%{name}%{version}.tgz
Copyright: GPL
Vendor: Wolfpack Development Team
Packager:  Stein Vrale <stein@terminator.net>
Group:     X11/Games

%description
XShipWars is a highly customizable and dynamic space-oriented gamming system
designed for play on the net. 
You create your own universes and build your own vessels with a huge
array of customizable settings. 
Graphics/video/sound themes for XShipWars are created by many talented
professional artists.

%package client
Summary: Space oriented network game system.
Group:   X11/games

%description client
X GUI client for XShipWars

%package server
Summary: Space oriented network game system.
Group:   Console/Daemons

%description server
The server and a sample universe file for XShipWars

%prep
%setup -n %{name}%{version}
./configure 

%build
make client_linux
make server_linux

%install
make client_install_unix
make server_install_unix

%files server
%dir /home/swserv
/home/swserv/*

%files client
%dir /usr/share/games/xshipwars
/usr/games/xsw
/usr/share/games/xshipwars/*
%doc README
%doc TODO
%doc INSTALL
%doc DOCS_NOW_ONLINE
%doc LICENSE
%doc CREDITS
