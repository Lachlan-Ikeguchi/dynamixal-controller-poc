{
  description = "A ROS 2 message to dynamixel interface";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    dynamixel = {
      url = "github:/ROBOTIS-GIT/DynamixelSDK";
      flake = false;
    };
  };

  outputs =
    { self
    , nixpkgs
    , flake-utils
    , dynamixel
    ,
    }:
    flake-utils.lib.eachSystem [ "aarch64-linux" "x86_64-linux" ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        dynamixel-sdk = pkgs.stdenv.mkDerivation {
          name = "dynamixel-sdk";
          version = "0.0.0";
          src = "${dynamixel}";
          patches = [ ./patches/ftdi-baud-rate.patch ];
          nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];
          configurePhase = ''
            runHook preConfigure
            mkdir -p $NIX_BUILD_TOP/dxl-build
            cd $NIX_BUILD_TOP/dxl-build
            cmake -DCMAKE_INSTALL_PREFIX=$out $NIX_BUILD_TOP/source/c++
            runHook postConfigure
          '';
          buildPhase = ''
            runHook preBuild
            cd $NIX_BUILD_TOP/dxl-build
            cmake --build .
            runHook postBuild
          '';
          installPhase = ''
            runHook preInstall
            cd $NIX_BUILD_TOP/dxl-build
            cmake --install .
            runHook postInstall
          '';
        };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            gcc
            cmake
            ninja
            dynamixel-sdk
          ];
        };

        packages.default = pkgs.stdenv.mkDerivation {
          name = "dynamixel-controller";
          src = self;
          nativeBuildInputs = [ pkgs.cmake pkgs.gcc pkgs.ninja ];
          buildInputs = [ dynamixel-sdk ];
          cmakeFlags = [
            "-DCMAKE_PREFIX_PATH=${dynamixel-sdk}"
            "-DCMAKE_BUILD_TYPE=Release"
          ];
        };

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/dynamixal-controller";
        };

        checks.default = pkgs.runCommand "smoke-test" {
          buildInputs = [ self.packages.${system}.default ];
        } ''
          dynamixal-controller --help
          touch $out
        '';
      });
}
