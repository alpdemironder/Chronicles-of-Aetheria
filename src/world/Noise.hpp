#pragma once

#include <cstdint>
#include <cmath>

namespace Aetheria {

class PerlinNoise {
public:
    PerlinNoise(uint32_t seed = 1337) {
        reseed(seed);
    }

    void reseed(uint32_t seed) {
        for (int i = 0; i < 256; ++i) p[i] = i;
        uint32_t state = seed;
        for (int i = 255; i > 0; --i) {
            state = state * 1664525u + 1013904223u;
            int j = state % (i + 1);
            int tmp = p[i];
            p[i] = p[j];
            p[j] = tmp;
        }
        for (int i = 0; i < 256; ++i) p[256 + i] = p[i];
    }

    float noise2D(float x, float y) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);

        float u = fade(x);
        float v = fade(y);

        int A = p[X] + Y;
        int B = p[X + 1] + Y;

        return lerp(v,
            lerp(u, grad2D(p[A], x, y), grad2D(p[B], x - 1, y)),
            lerp(u, grad2D(p[A + 1], x, y - 1), grad2D(p[B + 1], x - 1, y - 1))
        );
    }

    float fbm2D(float x, float y, int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            total += noise2D(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

    float noise3D(float x, float y, float z) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(w,
            lerp(v,
                lerp(u, grad3D(p[AA], x, y, z), grad3D(p[BA], x - 1, y, z)),
                lerp(u, grad3D(p[AB], x, y - 1, z), grad3D(p[BB], x - 1, y - 1, z))
            ),
            lerp(v,
                lerp(u, grad3D(p[AA + 1], x, y, z - 1), grad3D(p[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad3D(p[AB + 1], x, y - 1, z - 1), grad3D(p[BB + 1], x - 1, y - 1, z - 1))
            )
        );
    }

    float fbm3D(float x, float y, float z, int octaves = 3, float persistence = 0.5f, float lacunarity = 2.0f) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            total += noise3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

private:
    static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static float lerp(float t, float a, float b) { return a + t * (b - a); }

    static float grad2D(int hash, float x, float y) {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

    static float grad3D(int hash, float x, float y, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

    int p[512] = {};
};

} // namespace Aetheria
