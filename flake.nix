{
  description = "json_wrapper shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
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

            # libs 
            rapidjson
            capnproto
          ];

          shellHook = ''
            echo "CMake: $(cmake --version | head -n1)"
            echo "Ninja: $(ninja --version)"
          '';
        };
      });
    };
}
