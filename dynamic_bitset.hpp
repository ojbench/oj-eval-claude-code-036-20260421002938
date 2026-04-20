#ifndef DYNAMIC_BITSET_HPP
#define DYNAMIC_BITSET_HPP

#include <vector>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <algorithm>

namespace sjtu {

struct dynamic_bitset {
private:
    std::vector<uint64_t> data;
    std::size_t bit_size;

    static constexpr std::size_t BITS_PER_BLOCK = 64;

    std::size_t num_blocks() const {
        return (bit_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;
    }

public:
    // 默认构造函数，默认长度为 0
    dynamic_bitset() : bit_size(0) {}

    // 除非手动管理内存，否则 = default 即可
    ~dynamic_bitset() = default;

    /**
     * @brief 拷贝构造函数
     * 如果你用 std::vector 来实现，那么这个函数可以直接 = default
     * 如果你手动管理内存，则你可能需要自己实现这个函数
     */
    dynamic_bitset(const dynamic_bitset &) = default;

    /**
     * @brief 拷贝赋值运算符
     * 如果你用 std::vector 来实现，那么这个函数可以直接 = default
     * 如果你手动管理内存，则你可能需要自己实现这个函数
     */
    dynamic_bitset &operator = (const dynamic_bitset &) = default;

    // 初始化 bitset 的大小为 n ，且全为 0.
    dynamic_bitset(std::size_t n) : bit_size(n) {
        std::size_t blocks = num_blocks();
        data.resize(blocks, 0);
    }

    /**
     * @brief 从一个字符串初始化 bitset。
     * 保证字符串合法，且最低位在最前面。
     * 例如 a =  "0010"，则有:
     * a 的第 0 位是 0
     * a 的第 1 位是 0
     * a 的第 2 位是 1
     * a 的第 3 位是 0
     */
    dynamic_bitset(const std::string &str) : bit_size(str.size()) {
        std::size_t blocks = num_blocks();
        data.resize(blocks, 0);

        for (std::size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '1') {
                std::size_t block_idx = i / BITS_PER_BLOCK;
                std::size_t bit_idx = i % BITS_PER_BLOCK;
                data[block_idx] |= (1ULL << bit_idx);
            }
        }
    }

    // 访问第 n 个位的值，和 vector 一样是 0-base
    bool operator [] (std::size_t n) const {
        if (n >= bit_size) return false;
        std::size_t block_idx = n / BITS_PER_BLOCK;
        std::size_t bit_idx = n % BITS_PER_BLOCK;
        return (data[block_idx] >> bit_idx) & 1ULL;
    }

    // 把第 n 位设置为指定值 val
    dynamic_bitset &set(std::size_t n, bool val = true) {
        if (n >= bit_size) return *this;
        std::size_t block_idx = n / BITS_PER_BLOCK;
        std::size_t bit_idx = n % BITS_PER_BLOCK;

        if (val) {
            data[block_idx] |= (1ULL << bit_idx);
        } else {
            data[block_idx] &= ~(1ULL << bit_idx);
        }
        return *this;
    }

    // 在尾部插入一个位，并且长度加一
    /*
    补充说明：这里指的是高位，
    比如0010后面push_back(1)应该变为00101
    */
    dynamic_bitset &push_back(bool val) {
        std::size_t old_size = bit_size;
        bit_size++;

        std::size_t new_blocks = num_blocks();
        if (new_blocks > data.size()) {
            data.resize(new_blocks, 0);
        }

        if (val) {
            std::size_t block_idx = old_size / BITS_PER_BLOCK;
            std::size_t bit_idx = old_size % BITS_PER_BLOCK;
            data[block_idx] |= (1ULL << bit_idx);
        }

        return *this;
    }

    // 如果不存在 1 ，则返回 true。否则返回 false
    bool none() const {
        for (std::size_t i = 0; i < data.size(); ++i) {
            if (i < data.size() - 1) {
                if (data[i] != 0) return false;
            } else {
                // For the last block, only check valid bits
                std::size_t valid_bits = bit_size % BITS_PER_BLOCK;
                if (valid_bits == 0) valid_bits = BITS_PER_BLOCK;
                uint64_t mask = (valid_bits == BITS_PER_BLOCK) ? ~0ULL : ((1ULL << valid_bits) - 1);
                if ((data[i] & mask) != 0) return false;
            }
        }
        return true;
    }

    // 如果不存在 0 ，则返回 true。否则返回 false
    bool all() const {
        if (bit_size == 0) return true;

        for (std::size_t i = 0; i < data.size(); ++i) {
            if (i < data.size() - 1) {
                if (data[i] != ~0ULL) return false;
            } else {
                // For the last block, only check valid bits
                std::size_t valid_bits = bit_size % BITS_PER_BLOCK;
                if (valid_bits == 0) valid_bits = BITS_PER_BLOCK;
                uint64_t mask = (valid_bits == BITS_PER_BLOCK) ? ~0ULL : ((1ULL << valid_bits) - 1);
                if ((data[i] & mask) != mask) return false;
            }
        }
        return true;
    }

    // 返回自身的长度
    std::size_t size() const {
        return bit_size;
    }

    /**
     * 所有位运算操作均按照以下规则进行:
     * 取两者中较短的长度那个作为操作长度。
     * 换句话说，我们仅操作两者中重叠的部分，其他部分不变。
     * 在操作前后，bitset 的长度不应该发生改变。
     *
     * 比如 a = "10101", b = "1100"
     * a |= b 之后，a 应该变成 "11101"
     * b |= a 之后，b 应该变成 "1110"
     * a &= b 之后，a 应该变成 "10001"
     * b &= a 之后，b 应该变成 "1000"
     * a ^= b 之后，a 应该变成 "01101"
     * b ^= a 之后，b 应该变成 "0110"
     */

    // 或操作，返回自身的引用。     a |= b 即 a = a | b
    dynamic_bitset &operator |= (const dynamic_bitset &other) {
        std::size_t min_size = std::min(bit_size, other.bit_size);
        std::size_t min_blocks = (min_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

        for (std::size_t i = 0; i < min_blocks; ++i) {
            if (i < min_blocks - 1) {
                data[i] |= other.data[i];
            } else {
                // Handle the last block carefully
                std::size_t valid_bits = min_size % BITS_PER_BLOCK;
                if (valid_bits == 0) valid_bits = BITS_PER_BLOCK;
                uint64_t mask = (valid_bits == BITS_PER_BLOCK) ? ~0ULL : ((1ULL << valid_bits) - 1);
                data[i] = (data[i] & ~mask) | ((data[i] | other.data[i]) & mask);
            }
        }

        return *this;
    }

    // 与操作，返回自身的引用。     a &= b 即 a = a & b
    dynamic_bitset &operator &= (const dynamic_bitset &other) {
        std::size_t min_size = std::min(bit_size, other.bit_size);
        std::size_t min_blocks = (min_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

        for (std::size_t i = 0; i < min_blocks; ++i) {
            if (i < min_blocks - 1) {
                data[i] &= other.data[i];
            } else {
                // Handle the last block carefully
                std::size_t valid_bits = min_size % BITS_PER_BLOCK;
                if (valid_bits == 0) valid_bits = BITS_PER_BLOCK;
                uint64_t mask = (valid_bits == BITS_PER_BLOCK) ? ~0ULL : ((1ULL << valid_bits) - 1);
                data[i] = (data[i] & ~mask) | ((data[i] & other.data[i]) & mask);
            }
        }

        return *this;
    }

    // 异或操作，返回自身的引用。   a ^= b 即 a = a ^ b
    dynamic_bitset &operator ^= (const dynamic_bitset &other) {
        std::size_t min_size = std::min(bit_size, other.bit_size);
        std::size_t min_blocks = (min_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

        for (std::size_t i = 0; i < min_blocks; ++i) {
            if (i < min_blocks - 1) {
                data[i] ^= other.data[i];
            } else {
                // Handle the last block carefully
                std::size_t valid_bits = min_size % BITS_PER_BLOCK;
                if (valid_bits == 0) valid_bits = BITS_PER_BLOCK;
                uint64_t mask = (valid_bits == BITS_PER_BLOCK) ? ~0ULL : ((1ULL << valid_bits) - 1);
                data[i] = (data[i] & ~mask) | ((data[i] ^ other.data[i]) & mask);
            }
        }

        return *this;
    }

    /**
     * @brief 左移 n 位 。类似无符号整数的左移，最低位会补 0.
     * 例如 a = "1110"
     * a <<= 3 之后，a 应该变成 "0001110"
     * @return 返回自身的引用
     */
    dynamic_bitset &operator <<= (std::size_t n) {
        if (n == 0) return *this;

        std::size_t new_size = bit_size + n;
        std::size_t new_blocks = (new_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

        std::vector<uint64_t> new_data(new_blocks, 0);

        std::size_t block_shift = n / BITS_PER_BLOCK;
        std::size_t bit_shift = n % BITS_PER_BLOCK;

        if (bit_shift == 0) {
            // Simple block shift
            for (std::size_t i = 0; i < data.size(); ++i) {
                new_data[i + block_shift] = data[i];
            }
        } else {
            // Need to shift bits within blocks
            for (std::size_t i = 0; i < data.size(); ++i) {
                new_data[i + block_shift] |= (data[i] << bit_shift);
                if (i + block_shift + 1 < new_blocks) {
                    new_data[i + block_shift + 1] |= (data[i] >> (BITS_PER_BLOCK - bit_shift));
                }
            }
        }

        data = std::move(new_data);
        bit_size = new_size;

        return *this;
    }

    /**
     * @brief 右移 n 位 。类似无符号整数的右移，最低位丢弃。
     * 例如 a = "10100"
     * a >>= 2 之后，a 应该变成 "100"
     * a >>= 9 之后，a 应该变成 "" (即长度为 0)
     * @return 返回自身的引用
     */
    dynamic_bitset &operator >>= (std::size_t n) {
        if (n >= bit_size) {
            bit_size = 0;
            data.clear();
            return *this;
        }

        if (n == 0) return *this;

        std::size_t new_size = bit_size - n;
        std::size_t new_blocks = (new_size + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

        std::vector<uint64_t> new_data(new_blocks, 0);

        std::size_t block_shift = n / BITS_PER_BLOCK;
        std::size_t bit_shift = n % BITS_PER_BLOCK;

        if (bit_shift == 0) {
            // Simple block shift
            for (std::size_t i = 0; i < new_blocks; ++i) {
                new_data[i] = data[i + block_shift];
            }
        } else {
            // Need to shift bits within blocks
            for (std::size_t i = 0; i < new_blocks; ++i) {
                new_data[i] = data[i + block_shift] >> bit_shift;
                if (i + block_shift + 1 < data.size()) {
                    new_data[i] |= (data[i + block_shift + 1] << (BITS_PER_BLOCK - bit_shift));
                }
            }
        }

        data = std::move(new_data);
        bit_size = new_size;

        return *this;
    }

    // 把所有位设置为 1
    dynamic_bitset &set() {
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = ~0ULL;
        }

        // Clear extra bits in the last block
        if (bit_size > 0) {
            std::size_t valid_bits = bit_size % BITS_PER_BLOCK;
            if (valid_bits != 0) {
                uint64_t mask = (1ULL << valid_bits) - 1;
                data[data.size() - 1] &= mask;
            }
        }

        return *this;
    }

    // 把所有位取反
    dynamic_bitset &flip() {
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = ~data[i];
        }

        // Clear extra bits in the last block
        if (bit_size > 0) {
            std::size_t valid_bits = bit_size % BITS_PER_BLOCK;
            if (valid_bits != 0) {
                uint64_t mask = (1ULL << valid_bits) - 1;
                data[data.size() - 1] &= mask;
            }
        }

        return *this;
    }

    // 把所有位设置为 0
    dynamic_bitset &reset() {
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = 0;
        }
        return *this;
    }
};

}  // namespace sjtu

#endif  // DYNAMIC_BITSET_HPP
