    {type} Read_{formatted}(string objectKey) {
        {type} result = ({type})malloc(sizeof({underlying}));
        if(result == NULL) {
            cerr << "Error in memory allocation\n";
            exit(EXIT_FAILURE);
        }
        
        *result = Read_{underlyingFormatted}(objectKey);
        pointers.push_back(result);
        
        return result;
    }