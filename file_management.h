#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

void checkIfClosingAuction(char* AID);

void setupWorkspace();

int checkCredentials(char* UID, char* password);

int auctionExists(char* AID);

int auctionIsOwnedBy(char*UID, char* AID);

void registerUser(char* UID, char* password);

void unregisterUser(char* UID);

char* listMyAuctions(char* UID);

char* listMyBids(char* UID);

char* listAllAuctions();

void createAsset(char* filename, char* data, char* size);

char * createAuction(char* auction_name, char* host_UID, char* asset_fname, char* asset_data, char* asset_size, char* start_value, char* timeactive);

void closeAuction(char* AID);

char* showAsset(char* AID);

int bid(char* AID, char* value, char* UID);

char* showRecord(char* AID);

char* getNextAID();

#endif