/* Status and Packet Type Codes for Envy Packets */

#define PKT_TYPE_STATUS	0x01 //Information or statistics from a node
#define PKT_TYPE_CMD	0x02 //High level system command send to a node
#define PKT_TYPE_TASK	0x03 //Task instruction from master controller or completed response from node

#define STATUS_OK //Command or task completed successfully
#define STATUS_BOOT_OK	0x01 //Node came online with no problems
#define STATUS_SHUTTING_DOWN	0x02 //Node is shutting down
#define STATUS_KEEP_ALIVE	0x03 //Node is still alive
#define STATUS_STATISTIC	0x04 //Some kind of stastistic sent from node