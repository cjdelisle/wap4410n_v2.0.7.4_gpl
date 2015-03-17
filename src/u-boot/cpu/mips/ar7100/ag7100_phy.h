#ifndef _AG7100_PHY_H
#define _AG7100_PHY_H

#ifdef CFG_ATHR_PHY
#define ag7100_phy_setup(unit) do { \
if(!unit) \
        athr_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif
/*
#ifdef CFG_ATHRS26_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        athrs26_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VSC8201_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VSC8601_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VITESSE_8601_7395_PHY

#define ag7100_phy_setup(unit) do { \
if(unit) \
	vsc73xx_setup(unit); \
else \
	vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, &link, &fdx, &speed,0); \
else \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, &duplex, 0,0); \
else \
	duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, 0, &speed,0); \
else \
	speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif


#ifdef CFG_IP175B_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        ip_phySetup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=ip_phyIsUp(unit); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = ip_phyIsFullDuplex(unit); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = ip_phySpeed(unit); \
} while (0);

#endif

#ifdef CONFIG_ADMTEK_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        miiphy_reset("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif
*/
#endif /*_AG7100_PHY_H*/
