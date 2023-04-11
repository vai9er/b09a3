
//helper function that checks whether a CHAR* is an int
//if it is, return 1, else return 0
int isInt(char *str){
    int i = 0;
    while (str[i] != '\0'){
        if (isdigit(str[i]) == 0){
            return 0;
        }
        i++;
    }
    return 1;
}

//this function parses the command line arguments. It takes in the argc and argv from main, and the pointers to the variables that will be changed.
//it will change the variables to the values specified by the user, or to the default values if the user did not specify a value
//it will also check for invalid arguments and print an error message if it finds one
void parseArgs(int argc, char** argv, int* systemm, int* user, int* sequential, int* samples, int* tdelay, int* graphics) {
    int positionalArgIndex = 0;
    int sampleFlag = 0;
    int tdelFlag = 0;
    for (int i = 1; i < argc; i++) {
        //here we check for the system and user flags
        if (strcmp(argv[i], "--system") == 0) {
            *systemm = 1;
        } 
        else if (strcmp(argv[i], "--user") == 0) {
            *user = 1;
        } 
        //check for the sequential flag
        else if (strcmp(argv[i], "--sequential") == 0) {
            *sequential = 1;
        }

        else if (strcmp(argv[i], "--graphics") == 0) {
            *graphics = 1;
        }

        //check for the samples and tdelay flags
        else if (strncmp(argv[i], "--samples=", 10) == 0) {
            if(positionalArgIndex != 0){
                printf("Invalid argument: Cannot have both positional arguement and --samples/--tdelay\n");
                exit(1);
            }
            *samples = atoi(argv[i] + 10);
            sampleFlag = 1;
            //here we check if the user specified a value for samples that is less than 1
            if (*tdelay <1){
                printf("Invalid arguement: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for samples that is not an int
            if (isInt(argv[i] + 10) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
            

        }
        //here we check if the user specified a value for tdelay that is less than 1 
         else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
            if(positionalArgIndex != 0){
                printf("Invalid argument: Cannot have both positional arguement and --samples/--tdelay\n");
                exit(1);
            }
            *tdelay = atoi(argv[i] + 9);
            tdelFlag = 1;
            //here we check if the user specified a value for tdelay that is less than 1
            if (*tdelay <1){
                printf("Invalid arguement: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for tdelay that is not an int
            if (isInt(argv[i] + 9) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
        } 
        //here we check for positional arguments
        else if (positionalArgIndex == 0) {

            if (tdelFlag == 1 || sampleFlag == 1){
                printf("Invalid argument: Cannot have both positional arguement and --samples/--tdelay\n");
                exit(1);
            }
            int value = atoi(argv[i]);
            if (value <= 0) {
                //here we check if the user specified a value for samples that is less than 1
                printf("Invalid positional argument: %s\n", argv[i]);
                exit(1);
            }
            if (isInt(argv[i]) == 0){
                //here we check if the user specified a value for samples that is not an int
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
            *samples = value;
            positionalArgIndex++;
        } 
        //here we check for positional arguments
        else if (positionalArgIndex == 1) {
            if (tdelFlag == 1 || sampleFlag == 1){
                printf("Invalid argument: Cannot have both positional arguement and --samples/--tdelay\n");
                exit(1);
            }
            int value = atoi(argv[i]);
            if (value <= 0) {
                //here we check if the user specified a value for tdelay that is less than 1
                printf("Invalid positional argument: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for tdelay that is not an int
            if (isInt(argv[i]) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
            *tdelay = value;
            positionalArgIndex++;
        } else {
            printf("Invalid argument: %s\n", argv[i]);
            exit(1);
        }
    }
    //here we check if the user specified the correct number of positional arguments
    if (positionalArgIndex != 0 && positionalArgIndex != 2) {
        printf("Two positional arguments are required, but %d were provided\n", positionalArgIndex);
        exit(1);
    }
    
}
