/*
 * Boat Management System
 *
 * This program tracks boats at Nautical Ventures marina, including:
 * - Boats in slips ($12.50/foot/month)
 * - Boats on land for work ($14.00/foot/month)
 * - Boats on trailors ($25.00/foot/month)
 * - Boats in storage ($11.20/foot/month)
 *
 * The program loads boat data from a CSV file, allows the user to manage the inventory,
 * and saves the data back to the file when exiting.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 
 #define MAX_BOATS 120
 #define MAX_NAME_LENGTH 128
 #define MAX_BOAT_LENGTH 100
 #define MAX_SLIP_NUM 85
 #define MAX_STORAGE_SPACE 50
 
 /* Rates per foot per month */
 #define SLIP_RATE 12.50
 #define LAND_RATE 14.00
 #define TRAILOR_RATE 25.00
 #define STORAGE_RATE 11.20
 
 /* Location types for boats */
 typedef enum {
   SLIP,
   LAND,
   TRAILOR,
   STORAGE
 } LocationType;
 
 /* Location-specific information */
 typedef union {
   int slipNumber;       /* 1-85 */
   char bayLetter;       /* A-Z */
   char trailorTag[10];  /* License tag */
   int storageSpace;     /* 1-50 */
 } LocationInfo;
 
 /* Boat structure */
 typedef struct {
   char name[MAX_NAME_LENGTH];
   float length;
   LocationType locationType;
   LocationInfo locationInfo;
   float amountOwed;
 } Boat;
 
 /* Function prototypes */
 void displayWelcomeMessage();
 void displayExitMessage();
 void displayMenu();
 void loadBoatData(const char* filename, Boat** boats, int* boatCount);
 void saveBoatData(const char* filename, Boat** boats, int boatCount);
 int compareBoats(const void* a, const void* b);
 void displayInventory(Boat** boats, int boatCount);
 void addBoat(Boat** boats, int* boatCount, const char* boatData);
 void removeBoat(Boat** boats, int* boatCount);
 void acceptPayment(Boat** boats, int boatCount);
 void updateMonthlyCharges(Boat** boats, int boatCount);
 char* locationTypeToString(LocationType type);
 int findBoatByName(Boat** boats, int boatCount, const char* name);
 void freeAllBoats(Boat** boats, int boatCount);
 
 int main(int argc, char* argv[]) {
   Boat* boats[MAX_BOATS] = {NULL};
   int boatCount = 0;
   char choice;
   char inputBuffer[256];
   
   /* Check for correct number of command line arguments */
   if (argc != 2) {
     printf("Usage: %s <filename.csv>\n", argv[0]);
     return 1;
   }
   
   /* Load boat data from file */
   loadBoatData(argv[1], boats, &boatCount);
   
   /* Display welcome message */
   displayWelcomeMessage();
   
   /* Main program loop */
   do {
     displayMenu();
     
     if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
       choice = toupper(inputBuffer[0]);
       
       switch (choice) {
         case 'I':
           displayInventory(boats, boatCount);
           break;
         
         case 'A':
           printf("Please enter the boat data in CSV format                 : ");
           if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
             inputBuffer[strcspn(inputBuffer, "\n")] = '\0'; /* Remove newline */
             addBoat(boats, &boatCount, inputBuffer);
           }
           break;
         
         case 'R':
           removeBoat(boats, &boatCount);
           break;
         
         case 'P':
           acceptPayment(boats, boatCount);
           break;
         
         case 'M':
           updateMonthlyCharges(boats, boatCount);
           break;
         
         case 'X':
           break;
         
         default:
           printf("Invalid option %c\n\n", choice);
           break;
       }
     }
   } while (choice != 'X');
   
   /* Save boat data to file */
   saveBoatData(argv[1], boats, boatCount);
   
   /* Display exit message */
   displayExitMessage();
   
   /* Free allocated memory */
   freeAllBoats(boats, boatCount);
   
   return 0;
 }
 
 /* Display welcome message */
 void displayWelcomeMessage() {
   printf("\nWelcome to the Boat Management System\n");
   printf("-------------------------------------\n\n");
 }
 
 /* Display exit message */
 void displayExitMessage() {
   printf("\nExiting the Boat Management System\n");
 }
 
 /* Display menu options */
 void displayMenu() {
   printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
 }
 
 /* Load boat data from CSV file */
 void loadBoatData(const char* filename, Boat** boats, int* boatCount) {
   FILE* file = fopen(filename, "r");
   char buffer[256];
   
   /* Check if file opened successfully */
   if (file == NULL) {
     printf("Warning: Could not open file %s for reading.\n", filename);
     return;
   }
   
   *boatCount = 0;
   
   /* Read each line from file */
   while (fgets(buffer, sizeof(buffer), file) != NULL && *boatCount < MAX_BOATS) {
     buffer[strcspn(buffer, "\n")] = '\0'; /* Remove newline */
     
     /* Allocate memory for new boat */
     Boat* newBoat = (Boat*)malloc(sizeof(Boat));
     if (newBoat == NULL) {
       printf("Error: Memory allocation failed.\n");
       continue;
     }
     
     /* Parse CSV line */
     char* token;
     char* rest = buffer;
     
     /* Parse boat name */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       continue;
     }
     strncpy(newBoat->name, token, MAX_NAME_LENGTH - 1);
     newBoat->name[MAX_NAME_LENGTH - 1] = '\0';
     
     /* Parse boat length */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       continue;
     }
     newBoat->length = atof(token);
     
     /* Parse location type */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       continue;
     }
     
     if (strcmp(token, "slip") == 0) {
       newBoat->locationType = SLIP;
       
       /* Parse slip number */
       token = strtok_r(rest, ",", &rest);
       if (token == NULL) {
         free(newBoat);
         continue;
       }
       newBoat->locationInfo.slipNumber = atoi(token);
     } 
     else if (strcmp(token, "land") == 0) {
       newBoat->locationType = LAND;
       
       /* Parse bay letter */
       token = strtok_r(rest, ",", &rest);
       if (token == NULL || strlen(token) == 0) {
         free(newBoat);
         continue;
       }
       newBoat->locationInfo.bayLetter = token[0];
     } 
     else if (strcmp(token, "trailor") == 0) {
       newBoat->locationType = TRAILOR;
       
       /* Parse trailor tag */
       token = strtok_r(rest, ",", &rest);
       if (token == NULL) {
         free(newBoat);
         continue;
       }
       strncpy(newBoat->locationInfo.trailorTag, token, 9);
       newBoat->locationInfo.trailorTag[9] = '\0';
     } 
     else if (strcmp(token, "storage") == 0) {
       newBoat->locationType = STORAGE;
       
       /* Parse storage space number */
       token = strtok_r(rest, ",", &rest);
       if (token == NULL) {
         free(newBoat);
         continue;
       }
       newBoat->locationInfo.storageSpace = atoi(token);
     } 
     else {
       /* Invalid location type */
       free(newBoat);
       continue;
     }
     
     /* Parse amount owed */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       continue;
     }
     newBoat->amountOwed = atof(token);
     
     /* Add boat to array */
     boats[*boatCount] = newBoat;
     (*boatCount)++;
   }
   
   /* Close file */
   fclose(file);
   
   /* Sort boats by name */
   qsort(boats, *boatCount, sizeof(Boat*), compareBoats);
 }
 
 /* Save boat data to CSV file */
 void saveBoatData(const char* filename, Boat** boats, int boatCount) {
   FILE* file = fopen(filename, "w");
   
   /* Check if file opened successfully */
   if (file == NULL) {
     printf("Error: Could not open file %s for writing.\n", filename);
     return;
   }
   
   /* Write each boat to file */
   for (int i = 0; i < boatCount; i++) {
     Boat* boat = boats[i];
     
     fprintf(file, "%s,%.0f,%s,", 
             boat->name, 
             boat->length,
             locationTypeToString(boat->locationType));
     
     /* Write location-specific information */
     switch (boat->locationType) {
       case SLIP:
         fprintf(file, "%d", boat->locationInfo.slipNumber);
         break;
       case LAND:
         fprintf(file, "%c", boat->locationInfo.bayLetter);
         break;
       case TRAILOR:
         fprintf(file, "%s", boat->locationInfo.trailorTag);
         break;
       case STORAGE:
         fprintf(file, "%d", boat->locationInfo.storageSpace);
         break;
     }
     
     /* Write amount owed */
     fprintf(file, ",%.2f\n", boat->amountOwed);
   }
   
   /* Close file */
   fclose(file);
 }
 
 /* Compare boats by name (for qsort) */
 int compareBoats(const void* a, const void* b) {
   Boat* boatA = *(Boat**)a;
   Boat* boatB = *(Boat**)b;
   
   return strcasecmp(boatA->name, boatB->name);
 }
 
 /* Display inventory of boats */
 void displayInventory(Boat** boats, int boatCount) {
   for (int i = 0; i < boatCount; i++) {
     Boat* boat = boats[i];
     
     printf("%-20s %3.0f' ", boat->name, boat->length);
     
     /* Display location-specific information */
     switch (boat->locationType) {
       case SLIP:
         printf("%8s   # %2d", "slip", boat->locationInfo.slipNumber);
         break;
       case LAND:
         printf("%8s      %c", "land", boat->locationInfo.bayLetter);
         break;
       case TRAILOR:
         printf("%8s %6s", "trailor", boat->locationInfo.trailorTag);
         break;
       case STORAGE:
         printf("%8s   # %2d", "storage", boat->locationInfo.storageSpace);
         break;
     }
     
     /* Display amount owed */
     printf("   Owes $%7.2f\n", boat->amountOwed);
   }
   
   printf("\n");
 }
 
 /* Add a boat to the inventory */
 void addBoat(Boat** boats, int* boatCount, const char* boatData) {
   /* Check if maximum boats reached */
   if (*boatCount >= MAX_BOATS) {
     printf("Error: Maximum number of boats reached.\n\n");
     return;
   }
   
   /* Allocate memory for new boat */
   Boat* newBoat = (Boat*)malloc(sizeof(Boat));
   if (newBoat == NULL) {
     printf("Error: Memory allocation failed.\n\n");
     return;
   }
   
   /* Parse CSV line */
   char buffer[256];
   strncpy(buffer, boatData, sizeof(buffer) - 1);
   buffer[sizeof(buffer) - 1] = '\0';
   
   char* token;
   char* rest = buffer;
   
   /* Parse boat name */
   token = strtok_r(rest, ",", &rest);
   if (token == NULL) {
     free(newBoat);
     printf("Error: Invalid boat data format.\n\n");
     return;
   }
   strncpy(newBoat->name, token, MAX_NAME_LENGTH - 1);
   newBoat->name[MAX_NAME_LENGTH - 1] = '\0';
   
   /* Parse boat length */
   token = strtok_r(rest, ",", &rest);
   if (token == NULL) {
     free(newBoat);
     printf("Error: Invalid boat data format.\n\n");
     return;
   }
   newBoat->length = atof(token);
   
   /* Parse location type */
   token = strtok_r(rest, ",", &rest);
   if (token == NULL) {
     free(newBoat);
     printf("Error: Invalid boat data format.\n\n");
     return;
   }
   
   if (strcasecmp(token, "slip") == 0) {
     newBoat->locationType = SLIP;
     
     /* Parse slip number */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       printf("Error: Invalid boat data format.\n\n");
       return;
     }
     newBoat->locationInfo.slipNumber = atoi(token);
   } 
   else if (strcasecmp(token, "land") == 0) {
     newBoat->locationType = LAND;
     
     /* Parse bay letter */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL || strlen(token) == 0) {
       free(newBoat);
       printf("Error: Invalid boat data format.\n\n");
       return;
     }
     newBoat->locationInfo.bayLetter = token[0];
   } 
   else if (strcasecmp(token, "trailor") == 0) {
     newBoat->locationType = TRAILOR;
     
     /* Parse trailor tag */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       printf("Error: Invalid boat data format.\n\n");
       return;
     }
     strncpy(newBoat->locationInfo.trailorTag, token, 9);
     newBoat->locationInfo.trailorTag[9] = '\0';
   } 
   else if (strcasecmp(token, "storage") == 0) {
     newBoat->locationType = STORAGE;
     
     /* Parse storage space number */
     token = strtok_r(rest, ",", &rest);
     if (token == NULL) {
       free(newBoat);
       printf("Error: Invalid boat data format.\n\n");
       return;
     }
     newBoat->locationInfo.storageSpace = atoi(token);
   } 
   else {
     /* Invalid location type */
     free(newBoat);
     printf("Error: Invalid location type.\n\n");
     return;
   }
   
   /* Parse amount owed */
   token = strtok_r(rest, ",", &rest);
   if (token == NULL) {
     free(newBoat);
     printf("Error: Invalid boat data format.\n\n");
     return;
   }
   newBoat->amountOwed = atof(token);
   
   /* Add boat to array */
   boats[*boatCount] = newBoat;
   (*boatCount)++;
   
   /* Sort boats by name */
   qsort(boats, *boatCount, sizeof(Boat*), compareBoats);
 }
 
 /* Remove a boat from the inventory */
 void removeBoat(Boat** boats, int* boatCount) {
   char name[MAX_NAME_LENGTH];
   
   printf("Please enter the boat name                               : ");
   if (fgets(name, sizeof(name), stdin) != NULL) {
     name[strcspn(name, "\n")] = '\0'; /* Remove newline */
     
     /* Find boat index */
     int index = findBoatByName(boats, *boatCount, name);
     
     if (index == -1) {
       printf("No boat with that name\n\n");
       return;
     }
     
     /* Free boat memory */
     free(boats[index]);
     
     /* Shift remaining boats */
     for (int i = index; i < *boatCount - 1; i++) {
       boats[i] = boats[i + 1];
     }
     
     /* Update boat count */
     (*boatCount)--;
   }
 }
 
 /* Accept payment for a boat */
 void acceptPayment(Boat** boats, int boatCount) {
   char name[MAX_NAME_LENGTH];
   float payment;
   
   printf("Please enter the boat name                               : ");
   if (fgets(name, sizeof(name), stdin) != NULL) {
     name[strcspn(name, "\n")] = '\0'; /* Remove newline */
     
     /* Find boat index */
     int index = findBoatByName(boats, boatCount, name);
     
     if (index == -1) {
       printf("No boat with that name\n\n");
       return;
     }
     
     printf("Please enter the amount to be paid                       : ");
     char buffer[50];
     if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
       payment = atof(buffer);
       
       /* Check if payment amount is valid */
       if (payment > boats[index]->amountOwed) {
         printf("That is more than the amount owed, $%.2f\n\n", boats[index]->amountOwed);
         return;
       }
       
       /* Update amount owed */
       boats[index]->amountOwed -= payment;
     }
   }
 }
 
 /* Update monthly charges for all boats */
 void updateMonthlyCharges(Boat** boats, int boatCount) {
   for (int i = 0; i < boatCount; i++) {
     Boat* boat = boats[i];
     float monthlyCharge = 0.0;
     
     /* Calculate monthly charge based on location type */
     switch (boat->locationType) {
       case SLIP:
         monthlyCharge = boat->length * SLIP_RATE;
         break;
       case LAND:
         monthlyCharge = boat->length * LAND_RATE;
         break;
       case TRAILOR:
         monthlyCharge = boat->length * TRAILOR_RATE;
         break;
       case STORAGE:
         monthlyCharge = boat->length * STORAGE_RATE;
         break;
     }
     
     /* Update amount owed */
     boat->amountOwed += monthlyCharge;
   }
   
   printf("\n");
 }
 
 /* Convert location type to string */
 char* locationTypeToString(LocationType type) {
   switch (type) {
     case SLIP:
       return "slip";
     case LAND:
       return "land";
     case TRAILOR:
       return "trailor";
     case STORAGE:
       return "storage";
     default:
       return "unknown";
   }
 }
 
 /* Find a boat by name (case insensitive) */
 int findBoatByName(Boat** boats, int boatCount, const char* name) {
   for (int i = 0; i < boatCount; i++) {
     if (strcasecmp(boats[i]->name, name) == 0) {
       return i;
     }
   }
   
   return -1; /* Boat not found */
 }
 
 /* Free all allocated memory */
 void freeAllBoats(Boat** boats, int boatCount) {
   for (int i = 0; i < boatCount; i++) {
     free(boats[i]);
   }
 }