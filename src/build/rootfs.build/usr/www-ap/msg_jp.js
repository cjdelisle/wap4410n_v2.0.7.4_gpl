//public message
var msg_blank = "%sを空白にすることはできません。\n";
var msg_space = "%sには空白またはスペースは使用できません\n";
var msg_blank_in = "%sには空白は使用できません\n";
var msg_mis_len = "%sは少なくとも%s文字にする必要があります\n";
var msg_invalid = "%sに無効な文字が含まれています\n有効な文字は次のとおりです: \n%s\n\n";
var msg_check_invalid = "%sに無効な数値が含まれています\n";
var msg_greater = "%s %sより大きい値である必要があります。\n";
var msg_less = "%s %sより小さい値である必要があります。\n";
var msg_first = "第1";  // eg. First field of GW must match device IP
var msg_second = "第2";
var msg_third = "第3";
var msg_fourth = "第4";
var msg_invalidMAC = "%sは16進数の12文字(0～9およびA～F)で構成する必要があります。デリミタ(:または-)は任意です。第2ビットを奇数にすることはできません。\n";
var msg_invalidOUI = "%sは16進数の6文字(0～9およびA～F)で構成する必要があります。デリミタ(:または-)は任意です。\n";
var msg_validIP = "%sが無効です。有効な範囲は1.0.0.1～254.255.255.254です\n";
var msg_validMask = "%sが無効です。有効な範囲は0.0.0.0～255.255.255.255です\n";
var msg_validBroadcast = "%sが無効です。有効な範囲は1.0.0.1～255.255.255.255です\n";
var msg_validIP1 = "%sが無効です。最初の部分は[1-126]または[128-223]でなければなりません\n";
var msg_validIP2 = "%sが無効です。第2部分は[0-255]でなければなりません\n";
var msg_validIP3 = "%sが無効です。第3部分は[0-255]でなければなりません\n";
var msg_validIP4 = "%sが無効です。第4部分は[1-254]でなければなりません\n";
var msg_validMask1 = "%sが無効です。最初の部分は[192, 224, 240, 248, 252, 254, 255]でなければなりません\n";
var msg_validMask2 = "%sが無効です。第2部分は[0, 128, 192, 224, 240, 248, 252, 254, 255]でなければなりません\n";
var msg_validMask3 = "%sが無効です。第3部分は[0, 128, 192, 224, 240, 248, 252, 254, 255]でなければなりません\n";
var msg_validMask4 = "%sが無効です。第4部分は[0, 128, 192, 224, 240, 248, 252]でなければなりません\n"; //ip4 don't use 254
var msg_validMask = "%sが無効です。サブネットマスクは連続した値でなければなりません。\n";
var na_var = "該当なし";
var wmode_msg = "この画面は、ブリッジモードの間は使用できません";
var msg_invalid_email = "%sが無効です。\n";
var msg_trustedhostIP = "%s もう一度入力してください。";

//Setup.htm
var msg_ap_name = "アクセスポイントのホスト名が無効です。ホスト名に指定できるのは文字、数字、およびハイフンだけです。\n";
var msg_not_number = "名前は数字だけで構成することはできません。\n";
var msg_hyphen = "ホスト名の先頭または末尾にハイフンを指定することはできません。\n";
var msg_device_name = "アクセスポイントのデバイス名が無効です。\n";
var msg_ip_field = "IPアドレスの%sフィールド";
var msg_ip_address = "IPアドレス";
var msg_netmask = "ネットワークマスク";
var msg_gatewayip = "ゲートウェイIPアドレス";
var msg_DNS_ip = "DNS IPアドレス";
var msg_IPv6_Blank = "IPv6アドレスが無効です。アドレスを空白にすることはできません。\n";
var msg_IPv6_prelen = "IPv6アドレスが無効です。プレフィクス長が間違っています。\n";
var msg_IPv6_len = "IPv6アドレスが無効です。長さが間違っています。\n";
var msg_IPv6_col3 = "IPv6アドレスが無効です。':::' は使用できません。\n";
var msg_IPv6_col2 = "IPv6アドレスが無効です - '::' は1つしか含めることはできません。\n";
var msg_IPv6_col0 = "IPv6アドレスの形式が間違っています。アドレスの先頭を ':' にすることはできませんが、'::' は問題ない可能性があります。\n";
var msg_IPv6_col9 = "IPv6アドレスの形式が間違っています。アドレスの末尾を ':' にすることはできませんが、'::' は問題ない可能性があります。\n";
var msg_IPv6_dot = "IPv6アドレスが無効です - '.' は使用できません。\n";
var msg_IPv6_colNum = "IPv6アドレスが無効です - ':' の個数が間違っています。\n";
var msg_IPv6_hex4 = "IPv6アドレスが無効です - ':' で区切られた数が多すぎます。\n";
var msg_IPv6_dec = "IPv6アドレスが無効です - ':' の個数が間違っています。\n";
var msg_IPv6_dec2 = "IPv6アドレスが無効です。'.' で区切られた数が多すぎます\n";
var msg_IPv6_dot3 = "IPv6アドレスが無効です。'.' で区切られた数が多すぎます。\n";
var msg_IPv6_colNumdot = "IPv6アドレスが無効です。':' の個数が間違っています。\n";
var msg_IPv6_hex = "IPv6アドレスが無効です。数値が16進数ではありません。\n";
var msg_IPv6_pre = "IPv6ゲートウェイの形式が間違っています。アドレスにはプレフィクス長が必要です。\n";
var msg_IPv6_host = "IPv6ゲートウェイの形式が間違っています。アドレスにホスト情報がありません。\n";
var msg_IPv6_unspec = "IPv6アドレスをunspecified ipにすることはできません。\n";
var msg_IPv6_loopback = "IPv6アドレスをloopback ipにすることはできません。\n";
var msg_IPv6_muti = "IPv6アドレスをmulticast ipにすることはできません。\n";
var msg_ip_mask_mismatch = "サブネットマスク: \"IPアドレス\" が一致しません\n";
var msg_invalid_gateway = "ゲートウェイ: 無効です\n";
var msg_invalid_gateway_subnet = "ゲートウェイ: サブネットが異なります\n";

//SetupTime.htm
var msg_hour_invalid = "時間が無効です。有効な範囲は0～23です。\n";
var msg_min_invalid = "分が無効です。有効な範囲は0～59です。\n";
var msg_sec_invalid = "秒が無効です。有効な範囲は0～59です。\n";
var msg_date_invalid = "日付が無効です。";

//SetupAdvanced.htm
var msg_802Pass_invalid = "パスワードが無効です。有効な範囲は4～63です。\n";
var msg_redirect_invalid = "URLが無効です。\n";
var msg_8021x_bad_length = "802.1Xサプリカント名の長さが[1 - 63]の範囲ではありません";
var msg_8021x_bad_length_pwd = "802.1Xサプリカントパスワードの長さが[4 - 63]の範囲ではありません";
var msg_redirect_invalid_https = "HTTPリダイレクトではHTTPSをサポートできません。\n";

//Wireless.htm
var msg_invad_ssid0 = "SSID 1を空白にすることはできません。\n";
var msg_invad_ssid = "SSID名が '\\' で終わっていません。";
var same_ssid = "同じSSID名が見つかりました。\n";
var msg_wds_ssid = "WDSモードでは、1つのSSIDしか有効にできません。\n";

//WSecurity.htm
var msg_wep_pass = "パスフレーズエラーです。\n";
var msg_hexkey = "キーが無効です。\n16進数のキーに使用できる文字は0～9およびA～Fだけです。\nキーのサイズは10文字(64ビット)または26文字(128ビット)です。\n";
var msg_asciikey = "キーが無効です。\nキーのサイズは5文字(64ビット)または13文字(128ビット)です。\n";
var msg_maxpass = "パスフレーズの最大サイズ: ";
var msg_wpa_key = "WPA/PSKキー ";
var msg_key_error = "は、出力文字でなければなりません。\n";
var msg_keysize_error = "は、8～64文字でなければなりません。\n";
var msg_key_renew = "WPAキーのライフタイム";
var msg_key2_renew = "WPA2キーのライフタイム";
var msg_key3_renew = "RADIUSキーのライフタイム";
var msg_radius_port = "RADIUSポート";
var msg_invalid_ip = "IPアドレスが無効です。もう一度入力してください。\n";
var msg_radius_server1 = "プライマリ認証サーバのIPアドレス\n";
var msg_radius_server2 = "バックアップ認証サーバのIPアドレス\n";
var msg_radius_port1 = "プライマリ認証サーバのポート番号\n";
var msg_radius_port2 = "バックアップ認証サーバのポート番号\n";
var msg_radius = "RADIUSサーバを1つ入力する必要があります。\n";
var msg_r_login_key1 = "プライマリ認証サーバの共有暗号キー\n";
var msg_r_login_key2 = "バックアップ認証サーバの共有暗号キー\n";
var msg_r_key_size = "は、1～64文字でなければなりません。\n";
var msg_r_wep_key = "この新しいWEP設定により影響が発生し、以前のSSIDのWEP設定が新しい設定に同期されます。\n続行しますか?";

//WMACFilter.htm
var msg_mac = "MACアドレスのエラーです。";
var msg_exist_1 = "警告: MAC[";
var msg_exist_2 = "]アドレスはすでに存在するため、無視されます。\n";

//Administration.htm
var msg_usname = "ユーザ名の長さが[1 - 63]の範囲ではありません";
var msg_us_password = "パスワードの長さが[4 - 63]の範囲ではありません";
var msg_pw_nomatch = "パスワードのエントリが一致しません。もう一度入力してください。\n";
var msg_syscontact = "システムコンタクト先";
var msg_sysname = "デバイス名が無効です。\n";
var msg_syslocation = "システムロケーションが無効です。\n";
var msg_readComm = "読み取り専用コミュニティ";
var msg_writeComm = "読み取り/書き込みコミュニティ";
var msg_trapComm = "トラップコミュニティ";
var msg_snmp_TrustedHost_HightLow = "SNMPトラステッドホストの終了IPを開始IPより小さい値にすることはできません。\n";
var msg_snmp_Trustedhost_IPaddress = "SNMPトラステッドホストの開始および終了IPアドレス";
var msg_snmp_Trustedhost_endIP = "SNMPトラステッドホストの終了IPアドレス";
var msg_snmp_Trap_desthost = "SNMPトラップ宛先";

//Log.htm
var msg_slog_server = "SyslogサーバのIPアドレス";
var msg_emailInterval = "ログ時間のしきい値";
var msg_emailQlen = "ログキューの長さ";
var msg_smtpServer = "SMTPメールサーバ\n";
var msg_emailForLog = "Eメールアドレス ";
var msg_emailForReturn = "返信用Eメールアドレス";
var msg_smtpServerInvalid = "SMTPメールサーバが無効です。\n";
var msg_log_email = "アラートログ用のEメールアドレスが無効です。";

//VLAN.htm
var vlan_warning = "VLANを有効にした後、同じ管理VLAN上にあるホストだけがデバイスにアクセスできます。";
var msg_wmm_warning = "警告: WMMを無効にすると、現在の実装に対し、11nレートでスループットのパフォーマンスに影響します。\n";
var msg_select_vlan = "リストからVLAN IDを1つ選択してください。 ";
var msg_cant_add_more = "これ以上VLAN IDを追加できません。";
var msg_new_vlan = "VLANリストの新しいVLAN ID";
var msg_old_vlan = "このVLAN IDはすでにVLANリストに含まれています。";
var msg_native_vlan = "ネイティブVLAN ID";
var msg_mgt_vlan = "管理VLAN ID";

//AdvancedWSettings.htm
var msg_DTIM_invalid = "DTIM間隔が無効です。有効な範囲は1～255です。\n";
var msg_Frag_invalid = "フラグメンテーションのしきい値が無効です。有効な範囲は256～2346です。\n";
var msg_RTS_invalid = "RTSのしきい値が無効です。有効な範囲は1～2347です。\n";
var msg_Beacon_invalid = "ビーコン間隔が無効です。有効な範囲は20～1000です。\n";
var msg_Balance_invalid = "ロードバランスが無効です。有効な範囲は0～100です。\n";
var msg_Bandwith_invaild = "チャネル帯域幅が無効です。UC/URモードのAPは、20MHzでのみ動作します。\n";
var msg_GI_long_error = "ガード間隔が無効です。チャネル帯域幅が40MHzの場合、ロング(800ns)に設定することはできません。\n";
var msg_not_blank= "SSID %sの使用率のしきい値を空白にすることはできません。\n";

//ApMode.htm
var msg_invalid_ssid = "SSIDを空白にすることはできません。\n";
var msg_all_blank = "MACアドレスを空白にすることはできません。\n";
var msg_invaid_mac_zero = "MACアドレスが無効です。\n";
var msg_ap_addr = "AP MACアドレス";
var msg_oui_addr = "AP OUI";
var msg_same_mac = "同じMACアドレスが見つかりました。\n";
var msg_wire_list = "リストから正当なAPを1つ選択してください。\n";
//Diagnostics.htm
var msg_Ping_input = "アドレスが無効です。\n";
var msg_Packet_input = "パケットサイズが無効です。有効な範囲は32～65500です。\n ";
// factorydefaults.htm
var msg_confirmDefault = "警告!\n工場出荷時設定を復元すると、現在の設定がすべて消去されます。\nこの操作を実行しますか?";

// FirmwareUpgrade.htm  ( copy to FirmwareUpgrade.htm )
var msg_upgradefw = "続行しますか?\n既存のインターネット接続はすべて切断されます。";
var msg_nofile = "ファイル名が指定されていません。正しいファイルを選択してください。\n";
var msg_invalidfile = "無効なファイル名が指定されました。もう一度入力してください:(*.img)\n";
var finish_msg = "\ファームウェアのアップグレードが完了しました。\nこれでルータが起動します。" + 
"\nLEDを点検してルータの準備が完了していることを確認し、再接続してください。";
var disable_remote_msg = "リモートアップグレードは無効です。";
var not_in_range_msg = "お使いのIPが、許可されている範囲にありません。";
var msg_upgrade_fail = "アップグレードに失敗しました。";
var msg_upgrade_invalid_file = "イメージファイルが正しくありません。";
var msg_upgrade_old_version = "アップグレードする最新バージョンがありません";
var msg_invalidpath = "無効なファイルパスが指定されました。';' を含めることはできません。\n";

//ConfigManagement.htm
var msg_confirmDefault = "警告!\n工場出荷時設定を復元すると、現在の設定がすべて消去されます。\nこの操作を実行しますか?\n\n[OK]をクリックして続行するか、[キャンセル]をクリックして中止します。";
var msg_restoreCfg = "警告!\nコンフィギュレーションファイルから設定を復元すると、現在の設定はすべて消去されます。\nこの操作を実行しますか?\n\n[OK]をクリックして続行するか、[キャンセル]をクリックして中止します。";
var msg_nocfg_file = "ファイル名を空白にすることはできません";
var msg_invalidcfgfile = "無効なファイル名が指定されました。もう一度入力してください:(*.cfg)";

//CertManagement.htm
var msg_invalidcertfile = "無効なファイル名が指定されました。もう一度入力してください:(*.pem)";
var msg_importpem = "[OK]をクリックして続行するか、[キャンセル]をクリックして中止します。";

// SurveyWait.htm
var msg_loss_mainpage = "警告! メインページが失われました。";

// WpsSetup.htm
var msg_invalid_pin_value = "PINの値が無効です。\n";
var msg_invalid_pin_length = "PINの値の長さが無効です。\n";

// RogueLegal.htm
var msg_mac_address = "AP MACアドレス";
var msg_ap_oui = "AP OUI";

// utility.js
var msg_confirm_savepage = "このページは保存済みですか?";
var msg_confirm_password = "パスワードを確認してください。";
