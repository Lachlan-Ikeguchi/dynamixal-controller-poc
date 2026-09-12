{
  description = "A ROS 2 message to dynamixel interface";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
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
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
          ];

          configurePhase = ''
            mkdir -p /tmp/dxl-build
            cd /tmp/dxl-build
            cmake $NIX_BUILD_TOP/source/c++ -DCMAKE_INSTALL_PREFIX=$out
          '';

          buildPhase = ''
            cd /tmp/dxl-build
            cmake --build .
          '';

          installPhase = ''
            mkdir -p $out
            cd /tmp/dxl-build
            cmake --install .
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
          nativeBuildInputs = with pkgs; [
            cmake
            gcc
            ninja
          ];
          buildInputs = [ dynamixel-sdk ];
          cmakeFlags = [
            "-DCMAKE_PREFIX_PATH=${dynamixel-sdk}"
            "-DCMAKE_BUILD_TYPE=Release"
          ];
          phases = "installPhase";
          installPhase = ''
            mkdir -p $out/bin
            mkdir -p /tmp/build
            cd /tmp/build
            cmake $src -DCMAKE_PREFIX_PATH=${dynamixel-sdk} -DCMAKE_BUILD_TYPE=Release
            cmake --build .
            cp dynamixal-controller $out/bin/
          '';
        };
      });
}
