{
  description = "A ROS 2 message to dynamixel interface";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs { system = "aarch64-linux"; };
      dynamixel-sdk = pkgs.stdenv.mkDerivation rec {
        name = "dynamixel-sdk";
        src = pkgs.fetchFromGitHub {
          owner = "ROBOTIS-GIT";
          repo = "DynamixelSDK";
          rev = "2ded684dff05a40ac78d6a16105c6ddc1b3b9930";
          sha256 = "sha256-NJynAxKJH/VPyYTWbS+56yTKOmXyo5B7pXwiN2gnbXY=";
        };
        nativeBuildInputs = with pkgs; [ cmake gcc ninja ];
        
        # Override all phases to do our own thing
        phases = "installPhase";
        
        installPhase = ''
          mkdir -p $out
          # Create build dir in writable location
          mkdir -p /tmp/dxl-build
          cd /tmp/dxl-build
          cmake $src/c++ -DCMAKE_INSTALL_PREFIX=$out
          cmake --build .
          cmake --install .
        '';
      };
    in {
      devShells.aarch64-linux.default = pkgs.mkShell {
        packages = with pkgs; [ gcc cmake ninja dynamixel-sdk ];
      };

      packages.aarch64-linux.default = pkgs.stdenv.mkDerivation {
        name = "dynamixel-controller";
        src = self;
        nativeBuildInputs = with pkgs; [ cmake gcc ninja ];
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
    };
}
