#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

class BigInteger;

bool operator< (const BigInteger& n1, const BigInteger& n2);

bool operator== (const BigInteger& n1, const BigInteger& n2);

bool operator> (const BigInteger& n1, const BigInteger& n2);

bool operator!= (const BigInteger& n1, const BigInteger& n2);

bool operator<= (const BigInteger& n1, const BigInteger& n2);

bool operator>= (const BigInteger& n1, const BigInteger& n2);

BigInteger operator+ (const BigInteger& n1, const BigInteger& n2);

BigInteger operator- (const BigInteger& n1, const BigInteger& n2);

BigInteger operator* (const BigInteger& n1, const BigInteger& n2);

BigInteger operator/ (const BigInteger& n1, const BigInteger& n2);

BigInteger operator% (const BigInteger& n1, const BigInteger& n2);

int number_of_digits(int n, const int& radix) {
    if (n == 0) {
        return 1;
    }
    n = abs(n);
    int result = 0;
    int cur_deg = 0;
    int cur_pow = 1; 
    while (cur_pow <= n) {
        cur_pow *= radix;
        ++cur_deg;
    }
    return cur_deg;
}

int sign_of_number(const int& n) {
    if (n >= 0) return 1;
    return -1;
}

class BigInteger {

private:    
    int power_of_radix=4;
    int radix=10'000; //must be a power of ten.
    std::vector<int> rank;

    void delete_nulls() {
        while (rank.size() && rank.back() == 0) {
            rank.pop_back();
        }
    }

    void carryover() {
        int carry = 0;
        for (int i = 0; i < rank.size(); ++i) {
            int new_carry = (rank[i] + carry) / radix;
            rank[i] = (rank[i] + carry) % radix;
            carry = new_carry;
        }
        while (carry) {
            rank.push_back(carry % radix);
            carry /= radix;
        }
    }

    void reconstructor() {
        carryover();
        int sign = 1;
        if (rank.back() < 0) {
            sign = -1;
        }
        size_t position = 0;
        while (position < rank.size()) {
            if (sign_of_number(rank[position]) == sign || rank[position] == 0) {
                ++position;
                continue;
            }
            rank[position] += 10 * sign;
            ++position;
            while (sign_of_number(rank[position]) != sign || rank[position] == 0) {
                rank[position] += 9 * sign;
                ++position;
            }
            rank[position] -= 1 * sign;
            ++position;
        }
        delete_nulls();
    }

public:
    BigInteger(): rank({0}) {}

    BigInteger(int n) {
        while (n) {
            rank.push_back(n % radix);
            n /= radix;
        }
    }

    BigInteger(const BigInteger& other) = default;

    ~BigInteger() = default;

    int sign() const {
        return sign_of_number(rank.back());
    }
    
    int operator[](const int& k) const {
        if (rank.size() <= k) {
            return 0;
        }
        return rank[k];
    }

    int size() const {
        return rank.size();
    }

    BigInteger& operator+= (const BigInteger& other) {
        while (rank.size() < other.rank.size()) {
            rank.push_back(0);
        }
        int carry = 0;
        for (size_t i = 0; i < rank.size(); ++i) {
            int other_val = 0;
            if (i < other.rank.size()) {
                other_val = other.rank[i];
            }
            rank[i] += other_val + carry;
        }
        reconstructor();
        return *this;
    }

    BigInteger& operator*= (const BigInteger& other) {
        size_t self_size = rank.size();
        size_t other_size = other.rank.size();
        while (rank.size() != self_size + other_size - 1) {
            rank.push_back(0);
        }
        for (int i = rank.size() - 1; i >= 0; --i) {
            int start_value = rank[i];
            for (int j1 = 0; j1 <= i; ++j1) {
                int j2 = i - j1;
                rank[i] += rank[j1] * other.rank[j2];
            }
            rank[i] -= start_value;
        }
        reconstructor();
        return *this;
    }

    BigInteger& operator-= (const BigInteger& other) {
        while (rank.size() < other.rank.size()) {
            rank.push_back(0);
        }
        int carry = 0;
        for (size_t i = 0; i < rank.size(); ++i) {
            int other_val = 0;
            if (i < other.rank.size()) {
                other_val = other.rank[i];
            }
            rank[i] = other_val + carry;
        }
        reconstructor();
        return *this;
    }

    BigInteger& operator/= (const BigInteger& other) {
        BigInteger result;
        BigInteger cur_dividend;
        
        int carry = 0;
        for (int position = rank.size() - 1; position >= other.rank.size() - 1; --position) {
            cur_dividend *= radix;
            cur_dividend += rank[position];
            int quotient = (carry * radix + rank[position]) / other.rank.back();
            
        }
        return *this;
    }

    BigInteger& operator%= (const BigInteger& other) {
        return *this;
    }

    std::string toString() const {
        std::string result;
        if (rank.back() < 0) {
            result = "-";
        }
        for (int i = rank.size() - 1; i >= 0; --i) {
            if (i != rank.size() - 1) {
                result += std::string(power_of_radix - number_of_digits(rank[i], 10), '0');
            }
            result += std::to_string(abs(rank[i]));
        }
        return result;
    }

    void show_each_rank() const {
        for (int i = rank.size() - 1; i >= 0; i--) {
            std::cout << rank[i] << " ";
        }
        std::cout << '\n';
    }

    //пробразования в int/bool
    //инкремент/дикремент, постфикс/префикс

    friend std::istream& operator>> (std::istream& in, BigInteger& n);
};

std::ostream& operator<< (std::ostream& out, const BigInteger& n) {
    out << n.toString();
    return out;
}

std::istream& operator>> (std::istream& in, BigInteger& n) {
    n = 0;
    //some code
    return in;
}

BigInteger operator+ (const BigInteger& n1, const BigInteger& n2) {
    BigInteger sum(n1);
    sum += n2;
    return sum;
}

BigInteger operator- (const BigInteger& n1, const BigInteger& n2) {
    BigInteger difference(n1);
    difference -= n2;
    return difference;
}

BigInteger operator* (const BigInteger& n1, const BigInteger& n2) {
    BigInteger product(n1);
    product *= n2;
    return product;
}

BigInteger operator/ (const BigInteger& n1, const BigInteger& n2) {
    BigInteger division_result = BigInteger(n1);
    division_result /= n2;
    return division_result;
}

BigInteger operator% (const BigInteger& n1, const BigInteger& n2) {
    BigInteger mod = BigInteger(n1);
    mod %= n2;
    return mod;
}

bool operator< (const BigInteger& n1, const BigInteger& n2) {
    if (n1.sign() != n2.sign()) {
        return n1.sign() < n2.sign();
    }
    int maxsize = std::max(n1.size(), n2.size());
    return  n1[maxsize] < n2[maxsize];
}

bool operator== (const BigInteger& n1, const BigInteger& n2) {
    for (int rank = 0; rank < std::max(n1.size(), n2.size()); ++rank) {
        if (n1[rank] != n2[rank]) return false;
    } 
    return true;
}

bool operator> (const BigInteger& n1, const BigInteger& n2) {
    return !(n1 < n2) && !(n1 == n2);
}

bool operator!= (const BigInteger& n1, const BigInteger& n2) {
    return !(n1 == n2);
}

bool operator<= (const BigInteger& n1, const BigInteger& n2) {
    return n1 < n2 || n1 == n2;
}

bool operator>= (const BigInteger& n1, const BigInteger& n2) {
    return n1 > n2 || n1 == n2;
}
