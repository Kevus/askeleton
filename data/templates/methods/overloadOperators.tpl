ostream &operator<<(ostream &os, const {type} &object) {
    os << "{type} {\n";
//{insertions}
    os << "}";
    return os;
}

bool operator==(const {type}& a, const {type}& b) {
    return (
//{comparisons}
    );
}