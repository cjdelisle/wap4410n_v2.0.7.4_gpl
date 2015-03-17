//public message
var msg_blank = "%s can not be blank.\n";
var msg_space = "Blanks or spaces are not allowed in %s\n";
var msg_blank_in = "Blanks are not allowed in %s\n";
var msg_mis_len = "%s must be at least %s characters\n";
var msg_invalid = "Invalid character or characters in %s\nValid characters are: \n%s\n\n";
var msg_check_invalid = "%s contains an invalid number\n";
var msg_greater = "%s must be greater than %s.\n";
var msg_less = "%s must be less than %s.\n";
var msg_first = "First";  // eg. First field of GW must match device IP
var msg_second = "Second";
var msg_third = "Third";
var msg_fourth = "Fourth";
var msg_invalidMAC = "%s must be 12 Hex chars (0~9 and A~F) with optional delimiters (: or -),and the second bit is not a odd number.\n";
var msg_invalidOUI = "%s must be 6 Hex chars (0~9 and A~F) with optional delimiters (: or -)\n";
var msg_validIP = "%s is invalid. Valid range is 1.0.0.1 to 254.255.255.254\n";
var msg_validMask = "%s is invalid. Valid range is 0.0.0.0 to 255.255.255.255\n";
var msg_validBroadcast = "%s is invalid. Valid range is 1.0.0.1 to 255.255.255.255\n"
var msg_validIP1 = "%s is invalid, first part must be [1-126] or [128-223]\n";
var msg_validIP2 = "%s is invalid, second part must be [0-255]\n";
var msg_validIP3 = "%s is invalid, third part must be [0-255]\n";
var msg_validIP4 = "%s is invalid, fourth part must be [1-254]\n";
var msg_validMask1 = "%s is invalid, first part must be [192, 224, 240, 248, 252, 254, 255]\n";
var msg_validMask2 = "%s is invalid, second part must be [0, 128, 192, 224, 240, 248, 252, 254, 255]\n";
var msg_validMask3 = "%s is invalid, third part must be [0, 128, 192, 224, 240, 248, 252, 254, 255]\n";
var msg_validMask4 = "%s is invalid, fourth part must be [0, 128, 192, 224, 240, 248, 252]\n"; //ip4 don't use 254
var msg_validMask = "%s is invalid, The subnet mask has to be contiguous.\n";
var na_var = "n/a";
var wmode_msg = "This screen is not available while in Bridge mode";
var msg_invalid_email = "%s is invalid.\n";
var msg_trustedhostIP = "%s Please enter it again.";

//Setup.htm
var msg_ap_name = "Access Point Host Name is invalid. Host Name contains only letters, digits and hyphens.\n"
var msg_not_number = "Name may not consist solely of digits (numbers).\n";
var msg_hyphen = "Host Name can't begin or end with a hyphen.\n";
var msg_device_name = "Access Point Device Name is invalid.\n";
var msg_ip_field = "%s field of IP address";
var msg_ip_address = "IP Address";
var msg_netmask = "Network Mask";
var msg_gatewayip = "Gateway IP Address";
var msg_DNS_ip = "DNS IP Address";
var msg_IPv6_Blank = "Invalid IPv6 address. Address can not be blank.\n";
var msg_IPv6_prelen = "Invalid IPv6 address. Prefix length is in error.\n";
var msg_IPv6_len = "Invalid IPv6 address. Length is in error.\n";
var msg_IPv6_col3 = "Invalid IPv6 address. ':::' is not allowed.\n";
var msg_IPv6_col2 = "Invalid IPv6 address - only one '::' is allowed.\n";
var msg_IPv6_col0 = "Wrong IPv6 address format. Address cannot start with ':', but '::' maybe be OK.\n";
var msg_IPv6_col9 = "Wrong IPv6 address format. Address cannot end with ':', but '::' maybe be OK.\n";
var msg_IPv6_dot = "Invalid IPv6 address - '.' is not allowed.\n";
var msg_IPv6_colNum = "Invalid IPv6 address - the number of ':' is in error.\n";
var msg_IPv6_hex4 = "Invalid IPv6 address - the number divided by ':' is over.\n";
var msg_IPv6_dec = "Invalid IPv6 address- the number of ':' is in error.\n";
var msg_IPv6_dec2 = "Invalid IPv6 address. The number divided by '.' is in error \n";
var msg_IPv6_dot3 = "Invalid IPv6 address. The number divided by '.' is over.\n";
var msg_IPv6_colNumdot = "Invalid IPv6 address. The number of ':' is in error.\n";
var msg_IPv6_hex = "Invalid IPv6 address. The number is not hex.\n";
var msg_IPv6_pre = "Wrong IPv6 Gateway format. Address must have prefix length.\n";
var msg_IPv6_host = "Wrong IPv6 Gateway format. Address have not any Host infomation.\n";
var msg_IPv6_unspec ="IPv6 Address can not be unspecified ip.\n";
var msg_IPv6_loopback ="IPv6 Address can not be loopback ip.\n";
var msg_IPv6_muti = "IPv6 Address can not be multicast ip.\n";
var msg_ip_mask_mismatch = "Subnet Mask:mismatch \"IP address\"\n";
var msg_invalid_gateway = "Gateway: invalid!\n";
var msg_invalid_gateway_subnet = "Gateway: in different subnets\n";

//SetupTime.htm
var msg_hour_invalid = "Invalid hour, valid range is 0 to 23.\n";
var msg_min_invalid = "Invalid minute, valid range is 0 to 59.\n";
var msg_sec_invalid = "Invalid second, valid range is 0 to 59.\n";
var msg_date_invalid = "Invalid Date.";

//SetupAdvanced.htm
var msg_802Pass_invalid = "Invalid password, valid range is 4 to 63.\n";
var msg_redirect_invalid = "Invalid URL.\n"
var msg_8021x_bad_length = "The length of 802.1X Supplicant Name if out of range [1 - 63]";
var msg_8021x_bad_length_pwd = "The length of 802.1X Supplicant Password if out of range [4 - 63]";
var msg_redirect_invalid_https = "https can not support by http Redirect.\n";

//Wireless.htm
var msg_invad_ssid0 = "SSID 1 can not be blank.\n";
var msg_invad_ssid = "SSID name don't end of '\\'";
var same_ssid = "The same SSID name was be found.\n";
var msg_wds_ssid = "In WDS mode, only one SSID could be enabled.\n";

//WSecurity.htm
var msg_wep_pass = "passphrase error.\n";
var msg_hexkey = "Invalid Key. \nHex keys can only include the characters 0~9 and A~F.\nKey size is 10 chars (64bit) or 26 chars (128bit).\n";
var msg_asciikey = "Invalid Key. \nKey size is 5 chars (64bit) or 13 chars (128bit).\n";
var msg_maxpass = "Maximum Passphrase Size is ";
var msg_wpa_key = "WPA/PSK Key ";
var msg_key_error = "must be print characters.\n";
var msg_keysize_error = "must be from 8 to 64 characters.\n";
var msg_key_renew = "WPA Key Lifetime";
var msg_key2_renew = "WPA2 Key Lifetime";
var msg_key3_renew = "RADIUS Key Lifetime";
var msg_radius_port = "RADIUS Port";
var msg_invalid_ip = "Invalid IP address, please enter again.\n";
var msg_radius_server1 = "Primary Authentication Server IP address\n";
var msg_radius_server2 = "Backup Authentication Server IP address\n";
var msg_radius_port1 = "Primary Authentication Server Port number\n";
var msg_radius_port2 = "Backup Authentication Server Port number\n";
var msg_radius = "You must input one Radius Server.\n";
var msg_r_login_key1 = "Primary Authentication Server Shared Secret\n";
var msg_r_login_key2 = "Backup Authentication Server Shared Secret\n";
var msg_r_key_size = "must be from 1 to 64 characters.\n";
var msg_r_wep_key = "This new WEP setting will impact and sync previous SSID's WEP settings to the new setting.\nDo you wish to continue?" ;

//WMACFilter.htm
var msg_mac = "MAC address error.";
var msg_exist_1 = "Warning: MAC ";
var msg_exist_2 = " already exists, it will be ignored.\n";

//Administration.htm
var msg_usname = "The length of Username if out of range [1 - 63]";
var msg_us_password = "The length of Password is out of range [4 - 63]";
var msg_pw_nomatch = "The passwords entries do not match, please enter again.\n";
var msg_syscontact = "System Contact";
var msg_sysname = "Device Name is invalid.\n";
var msg_syslocation = "System Location is invalid.\n";
var msg_readComm = "Read Only Community";
var msg_writeComm = "Read/Write Community";
var msg_trapComm = "Trap Community";
var msg_snmp_TrustedHost_HightLow = "SNMP Trusted Host End IP could not be less than Start IP.\n";
var msg_snmp_Trustedhost_IPaddress = "SNMP Trusted Host Start and End IP Address";
var msg_snmp_Trustedhost_endIP = "SNMP Trusted Host End IP Address";
var msg_snmp_Trap_desthost = "SNMP Trap-Destination";

//Log.htm
var msg_slog_server = "SysLog Server IP Address";
var msg_emailInterval = "Log Time Threshold";
var msg_emailQlen = "Log Queue Length";
var msg_smtpServer = "SMTP Mail Server\n";
var msg_emailForLog = "Email Address ";
var msg_emailForReturn = "Return Email Address";
var msg_smtpServerInvalid = "SMTP Mail Server is invalid.\n";
var msg_log_email = "Email Address for Alert Logs is invalid.";

//VLAN.htm
var vlan_warning = "After enabling VLAN, only host who is on the same management VLAN can access the device.";
var msg_wmm_warning = "Warning:Disabling WMM will affect throughput performance at 11n rates for current implementation.\n";
var msg_select_vlan = "Please select one VLAN ID from the list.";
var msg_cant_add_more = "Cannot add more VLAN ID.";
var msg_new_vlan = "New VLAN ID for VLAN List";
var msg_old_vlan = "This VLAN ID is already in VLAN List.";
var msg_native_vlan = "Native VLAN ID";
var msg_mgt_vlan = "Management VLAN ID";

//AdvancedWSettings.htm
var msg_DTIM_invalid = "Invalid DTIM Interval, valid range is 1 to 255.\n";
var msg_Frag_invalid = "Invalid Fragmentation Threshold, valid range is 256 to 2346.\n";
var msg_RTS_invalid = "Invalid RTS Threshold, valid range is 1 to 2347.\n";
var msg_Beacon_invalid = "Invalid Beacon Interval, valid range is 20 to 1000.\n";
var msg_Balance_invalid = "Invalid Balance, valid rang is 0 to 100.\n";
var msg_Bandwith_invaild = "Invalid Channel Bandwith. In UC/UR mode AP only work in 20MHz.\n";
var msg_GI_long_error = "Invalid Guard Interval,In Channel Bandwith is 40MHz mode can not set long(800ns).\n";
var msg_not_blank="SSID %s 's utilization threshold can not be blank.\n";

//ApMode.htm
var msg_invalid_ssid = "SSID can not be blank.\n";
var msg_all_blank = "The MAC Address could not be blank.\n"
var msg_invaid_mac_zero = "Invalid MAC Address.\n";
var msg_ap_addr = "AP MAC Address";
var msg_oui_addr = "AP OUI";
var msg_same_mac = "The same MAC Address was be found.\n";
var	msg_wire_list="Please select one legal AP from the list.\n";
//Diagnostics.htm
var msg_Ping_input = "Invalid address.\n";
var msg_Packet_input = "Invalid Packet size, valid range is 32 - 65500.\n "
// factorydefaults.htm
var msg_confirmDefault = "Warning!\nRestoring Factory Default Settings will erase all the current settings.\nAre you sure you wish to do this?";

// FirmwareUpgrade.htm  ( copy to FirmwareUpgrade.htm )
var msg_upgradefw = "Continue?\nAll existing Internet connections will be terminated.";
var msg_nofile = "No filename provided. Please select the correct file.\n";
var msg_invalidfile = "Invalid filename provided. please enter again:(*.img)\n";
var finish_msg = "\Firmware Upgrade completed. \nRouter will now restart." + 
"\nPlease check LEDs to see if Router is ready, then re-connect.";
var disable_remote_msg="Remote upgrade is disabled.";
var not_in_range_msg="Your IP not in allowed range.";
var msg_upgrade_fail = "Upgrade has failed!";
var msg_upgrade_invalid_file = "Incorrect image file!" ;
var msg_upgrade_old_version = "There is no latest version to upgrade";
var msg_invalidpath = "Invalid file path provided.  ';' is not allowed\n";

//ConfigManagement.htm
var msg_confirmDefault = "Warning!\nRestoring Factory Default Settings will erase all the current settings.\nAre you sure you wish to do this?\n\nClick OK to continue, Cancel to abort.";
var msg_restoreCfg = "Warning!\nRestoring settings from a config file will erase all the current settings.\nAre you sure you wish to do this?\n\nClick OK to continue, Cancel to abort.";
var msg_nocfg_file = "Filename can not be blank";
var msg_invalidcfgfile = "Invalid filename provided. please enter again:(*.cfg)";

//CertManagement.htm
var msg_invalidcertfile = "Invalid filename provided. please enter again:(*.pem)";
var msg_importpem = "Click OK to continue, Cancel to abort.";

// SurveyWait.htm
var msg_loss_mainpage = "Warning! Loss of main page!";

// WpsSetup.htm
var msg_invalid_pin_value = "Invalid PIN value.\n";
var msg_invalid_pin_length = "Invlid PIN value length.\n";

// RogueLegal.htm
var msg_mac_address = "AP MAC Address";

// utility.js
var msg_confirm_savepage = "Did you save this page?";
var msg_confirm_password = "Re-Confirm the password!";
