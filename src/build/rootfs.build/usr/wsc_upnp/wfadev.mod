<?xml version="1.0"?>
<root xmlns="urn:schemas-upnp-org:device-1-0">
    <specVersion>
        <major>1</major>
        <minor>0</minor>
    </specVersion>
    <URLBase>http://@IPADDR#:@UPNP_PORT#</URLBase>
    <device>
        <deviceType>urn:schemas-wifialliance-org:device:WFADevice:1</deviceType>
        <friendlyName>@MODEL_NAME#</friendlyName>
        <manufacturer>@OEM_NAME#</manufacturer>
        <manufacturerURL>@OEM_URL#</manufacturerURL>
        <modelDescription>@MODEL_DESC#</modelDescription>
        <modelName>@MODEL_NAME#</modelName>
        <modelNumber>@MODEL_NUM#</modelNumber>
        <modelURL>@OEM_URL#</modelURL>
        <serialNumber>@SERIAL_NUM#</serialNumber>
        <UDN>uuid:@UUID_WSC#</UDN>
        <UPC>@MODEL_NAME#</UPC>
        <serviceList>
            <service>
                <serviceType>urn:schemas-wifialliance-org:service:WFAWLANConfig:1</serviceType>
                <serviceId>urn:wifialliance-org:serviceId:WFAWLANConfig1</serviceId>
                <SCPDURL>/wfawlcfg_1.0.xml</SCPDURL>
                <controlURL>/upnp/control/WFAWLANConfig1</controlURL>
                <eventSubURL>/upnp/event/WFAWLANConfig1</eventSubURL>
            </service>
            <!-- Declarations for other services added by UPnP vendor (if any) go here -->
        </serviceList>
        <!-- 
        <deviceList>
            Description of embedded devices added by UPnP vendor (if any) go here
        </deviceList>
        -->
        <presentationURL>http://@IPADDR#/index.htm</presentationURL>
    </device>
</root>
