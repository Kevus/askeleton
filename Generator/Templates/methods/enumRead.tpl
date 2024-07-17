    {type} Read_{formatted}(string objectKey) {
        try {
            return static_cast<{type}>(std::stoi(readObject(objectKey)));
        } catch (const std::exception &e) {
            cerr << "Please, check the value of " << objectKey
                << ". The conversion is invalid: " << e.what()
                << "\nDefault value is used instead\n";
            return {};
        }
    }