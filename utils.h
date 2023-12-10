#ifndef UTILS_H
#define UTILS_H

char* intToString(int num);

char* pathClientFolder(char* UID);

char* pathClientMain(char* UID);

char* pathClientBidder(char* UID);

char* pathClientHost(char* UID);



char* pathAuctionFolder(char* AID);

char* pathAuctionMain(char* AID);

char* pathAuctionBids(char* AID);

char* pathAuctionEnd(char* AID);

char* pathAsset(char*asset_fname);




char* getCurrentDateTime();




char* getAuctionHostUID(char* AID);

char* getAuctionAssetFName(char* AID);

char* getAuctionStartValue(char* AID);

char* getAuctionStartDateTime(char* AID);

char* getAuctionTimeactive(char* AID);

char* getAuctionName(char* AID);

char* computeTimeDifference(char* datetime1, char* datetime2);

char* getAuctionRealDuration(char* AID);

char* getAuctionStatus(char* AID);

char* getAuctionEndDatetime(char* AID);

char* getClientPassword(char* UID);



char* getAssetData(char* AID);

char* getAssetSize(char* AID);




int getLastBidValue(char* filename);

int validateBid(char* AID, char* value);



char* getBidsInfo(char* AID);

#endif