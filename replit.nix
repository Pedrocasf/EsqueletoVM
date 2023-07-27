{ pkgs }: {
	deps = [
		pkgs.rustup
  pkgs.unixtools.xxd
  pkgs.cmake
  pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
    pkgs.python310
	];
}