{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      forAllSystems = nixpkgs.lib.genAttrs [
        "x86_64-linux"
        "aarch64-linux"
      ];
    in
    {
      formatter = forAllSystems (system: nixpkgs.legacyPackages.${system}.nixfmt-rfc-style);
      packages = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "psfeditor"; # Should be downcase if importing in NixOS/nixpkgs
            version = "1.0.0";
            src = pkgs.lib.cleanSource self;
            buildInputs = [ pkgs.qt5.qtbase ];
            nativeBuildInputs = with pkgs.qt5; [
              qmake
              wrapQtAppsHook
            ];
            installPhase = ''
              runHook preInstall
              install -d $out/bin
              install ./PSFEditor $out/bin/
              runHook postInstall
            '';
            meta = with pkgs; {
              mainProgram = "PSFEditor";
              license = lib.licenses.bsd3;
              platforms = lib.platforms.linux;
            };
          };
        }
      );
      apps = forAllSystems (system: {
        default = {
          type = "app";
          program = nixpkgs.lib.getExe self.packages.${system}.default;
        };
      });
    };
}
