#ifndef UTILS_H
#define UTILS_H

#define AUCTION_NAME_MAX_LEN 10
#define DATETIME_MAX_LEN 20
#define FNAME_MAX_LEN 24
#define UID_MAX_LEN 20 //FIXME
#define MAX_CLIENT_PATH_LEN 20 + 4 + 8 
#define MAX_AUCTION_PATH_LEN AUCTION_NAME_MAX_LEN + 4 + 9
#define DATE_TIME_MAX_LEN 20
#define PASSWORD_MAX_LEN 8

char* intToString(int num);

char* pathClientFolder(char* UID);

char* pathClientMain(char* UID);

char* pathClientBidder(char* UID);

char* pathClientHost(char* UID);



char* pathAuctionFolder(char* auction_name);

char* pathAuctionMain(char* auction_name);

char* pathAuctionBids(char* auction_name);

char* pathAuctionEnd(char* auction_name);

char* pathAsset(char*asset_fname);




char* getCurrentDateTime();




char* getAuctionHostUID(char* auction_name);

char* getAuctionAssetFName(char* auction_name);

char* getAuctionStartValue(char* auction_name);

char* getAuctionStartDateTime(char* auction_name);

char* getAuctionTimeactive(char* auction_name);

char* computeTimeDifference(char* datetime1, char* datetime2);

char* getAuctionRealDuration(char* auction_name);

char* getAuctionStatus(char* auction_name);

char* getAuctionEndDatetime(char* auction_name);

char* getClientPassword(char* UID);



char* getAssetData(char* auction_name);

char* getAssetSize(char* auction_name);




int getLastBidValue(char* filename);

int validateBid(char* auction_name, char* value);



char* getBidsInfo(char* auction_name);

#endif