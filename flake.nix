{
  description = "C++ Shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    # seastar.url = "github:wyattgill9/seastar-flake/main";
  };
  
  outputs = { self, nixpkgs }:
    let
      systems = ["x86_64-linux" "aarch64-darwin"];
      
      eachSystem = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = nixpkgs.legacyPackages.${system};
      });
    in
    {
      devShells = eachSystem ({ pkgs }: {
        default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          buildInputs = with pkgs; [           
            cmake
            ninja
            gdb
          ];

          shellHook = ''
            echo "$(cmake --version | head -n1)"
            echo "ninja version $(ninja --version)"
          '';
        };
      });
    };
}
