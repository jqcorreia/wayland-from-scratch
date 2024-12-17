{
  description = "Wayland dev Environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        packages = with pkgs; [
          wayland
          wayland-scanner
          wayland-protocols
          meson
          pkg-config
          ninja
          libschrift
        ];
        shellHook = "zsh";
        name = "TUDO dev shell";
      };
    };
}
