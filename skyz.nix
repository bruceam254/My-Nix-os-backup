  { config, lib, pkgs, ... }:

  {
    # ===========================================================================
    # Skyz' User Configuration
    # ===========================================================================
    users.users."skyz" = {
      isNormalUser = true; #creates a home dir and sets up a standard user account
      description = "skyz";
      shell = pkgs.fish;
      extraGroups = [ "networkmanager" "wheel" ]; #network | #admin
      packages = with pkgs; [

      ];
    };



  }
