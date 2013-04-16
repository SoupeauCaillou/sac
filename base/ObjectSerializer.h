#include <sstream>

template <class T>
class ObjectSerializer {
    public:
        static T string2object(const std::string & value) {
            std::istringstream iss(value);

            T res;
            iss >> res;

            return res;
        }

        static std::string object2string(T value) {
            std::stringstream ss;

            ss << value;

            return ss.str();
        }
};
