// Asteroids.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "olcConsoleGameEngine.h"
using namespace std;

class Asteroids : public olcConsoleGameEngine {
    
public:
    Asteroids() {
        m_sAppName = L"Asteroids";
    }

private:
    struct sSpaceObject {
        float x;
        float y;
        float vx;
        float vy;
        int nSize;
        float angle;
    };

    vector<sSpaceObject> asteroids;
    vector<sSpaceObject> bullets;
    sSpaceObject player;
    vector<pair<float, float>> modelShip;
    vector<pair<float, float>> modelAsteroids;
    bool bDead = false;
    int nScore = 0;

protected:
    virtual bool OnUserCreate() {
        modelShip = {
            { 0.0f, -5.0f },
            { -2.5f, +2.5f },
            { +2.5f, +2.5f }
        };  // A simple Isoceles Triangle

        int verts = 20;
        for (int i = 0; i < verts; i++) {
            //float radius = 1.0f;
            float radius = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
            float a = ((float)i / (float)verts) * 6.28318f;  // 2 * PI
            modelAsteroids.push_back(make_pair(radius * sinf(a), radius * cosf(a)));
        }

        ResetGame();

        return true;
    }

    virtual bool OnUserUpdate(float fElapsedTime) {
        // Reset game if player is dead
        if (bDead) {
            ResetGame();
        }

        // Clear screen
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

        /* Controls */
        // Steer
        if (m_keys[VK_LEFT].bHeld)
            player.angle -= 5.0f * fElapsedTime;
        if (m_keys[VK_RIGHT].bHeld)
            player.angle += 5.0f * fElapsedTime;

        // Thrust
        if (m_keys[VK_UP].bHeld) {
            // ACCELERATION changes VELOCITY (with respect to time)
            player.vx += sin(player.angle) * 20.0f * fElapsedTime;
            player.vy += -cos(player.angle) * 20.0f * fElapsedTime;
        }

        // VELOCITY changes POSITION (with respect to time)
        player.x += player.vx * fElapsedTime;
        player.y += player.vy * fElapsedTime;

        // Keep ship in gamespace
        WrapCoordinates(player.x, player.y, player.x, player.y);

        // Check ship collision with asteroids
        for (auto& asteroid : asteroids) {
            if (IsPointInsideCircle(asteroid.x, asteroid.y, asteroid.nSize, player.x, player.y)) {
                bDead = true;   // Uh oh...
            }
        }

        // Fire Bullet in direction of player
        if (m_keys[VK_SPACE].bReleased) {
            bullets.push_back({ player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 0, 0 });
        }

        // Update and draw asteroids
        for (auto& asteroid : asteroids) {
            asteroid.x += asteroid.vx * fElapsedTime;
            asteroid.y += asteroid.vy * fElapsedTime;
            asteroid.angle += 0.5f * fElapsedTime;
            WrapCoordinates(asteroid.x, asteroid.y, asteroid.x, asteroid.y);
            DrawWireFrameModel(modelAsteroids, asteroid.x, asteroid.y, asteroid.angle, asteroid.nSize, FG_YELLOW);
        }

        // Temporary vector to store new asteroids
        vector<sSpaceObject> newAsteroids;

        // Update and draw bullets
        for (auto& bullet : bullets) {
            bullet.x += bullet.vx * fElapsedTime;
            bullet.y += bullet.vy * fElapsedTime;
            WrapCoordinates(bullet.x, bullet.y, bullet.x, bullet.y);
            Draw(bullet.x, bullet.y);

            // Check collision with asteroids
            for (auto& asteroid : asteroids) {
                if (IsPointInsideCircle(asteroid.x, asteroid.y, asteroid.nSize, bullet.x, bullet.y)) {
                    // Asteroid hit (collison occurred)
                    bullet.x = -100;

                    if (asteroid.nSize > 4) {
                        // Create two child asteroids
                        float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
                        float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
                        newAsteroids.push_back({ asteroid.x, asteroid.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), (int)asteroid.nSize >> 1, 0.0f });
                        newAsteroids.push_back({ asteroid.x, asteroid.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), (int)asteroid.nSize >> 1, 0.0f });
                    }

                    asteroid.x = -100;
                    nScore += 100;
                }
            }
        }

        // Append new asteroids to existing vector
        for (auto asteroid : newAsteroids) {
            asteroids.push_back(asteroid);
        }

        // Remove off screen bullets
        if (bullets.size() > 0) {
            auto i = remove_if(bullets.begin(), bullets.end(), [&](sSpaceObject o) {
                return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() || o.y >= ScreenHeight());
            });

            if (i != bullets.end()) {
                bullets.erase(i);
            }
        }

        // Remove off screen asteroids
        if (asteroids.size() > 0) {
            auto i = remove_if(asteroids.begin(), asteroids.end(), [&](sSpaceObject o) {
                return o.x < 0;
            });

            if (i != asteroids.end()) {
                asteroids.erase(i);
            }
        }

        // Destroyed all asteroids ingame
        if (asteroids.empty()) {
            nScore += 1000;
            asteroids.clear();
            bullets.clear();

            // Add two new asteroids, but in a place where the player is not, we'll simply
            // add them 90 degrees left and right to the player, their coordinates will
            // be wrapped by th enext asteroid update
            asteroids.push_back({ 30.0f * sinf(player.angle - 3.14159f / 2.0f) + player.x,
                                  30.0f * cosf(player.angle - 3.14159f / 2.0f) + player.y,
                                  10.0f * sinf(player.angle), 
                                  10.0f * cosf(player.angle),
                                  (int)16, 0.0f });

            asteroids.push_back({ 30.0f * sinf(player.angle + 3.14159f / 2.0f) + player.x,
                                  30.0f * cosf(player.angle + 3.14159f / 2.0f) + player.y,
                                  10.0f * sinf(-player.angle), 
                                  10.0f * cosf(-player.angle), 
                                  (int)16, 0.0f });
        }

        // Draw Ship
        DrawWireFrameModel(modelShip, player.x, player.y, player.angle, 1.25f);

        // Draw Score
        DrawString(2, 2, L"SCORE: " + to_wstring(nScore));

        return true;
    }

    bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y) {
        return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
    }

    void DrawWireFrameModel(const vector<pair<float, float>>& vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE) {
        // pair.first = x coordinate
        // pair.second = y coordinate

        // Create translated model vector of coordinate pairs
        vector<pair<float, float>> vecTransformedCoordinates;
        int verts = vecModelCoordinates.size();
        vecTransformedCoordinates.resize(verts);

        //// Rotate
        //for (int i = 0; i < verts; i++) {
        //    vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
        //    vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
        //}

        //// Scale
        //for (int i = 0; i < verts; i++) {
        //    vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
        //    vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
        //}

        //// Translate
        //for (int i = 0; i < verts; i++) {
        //    vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
        //    vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
        //}

        // Update Rotation, Scale & Translation
        for (int i = 0; i < verts; i++) {
            // Rotate
            vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
            vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);

            // Scale
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;

            // Translate
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
        }

        // Draw Closed Polygon
        for (int i = 0; i < verts + 1; i++) {
            int j = (i + 1);
            DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
                    vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
        }
    }

    void WrapCoordinates(float ix, float iy, float& ox, float& oy) {
        ox = ix;
        oy = iy;

        if (ix < 0.0f) ox = ix + (float)ScreenWidth();
        if (ix >= (float)ScreenWidth()) ox = ix - (float)ScreenWidth();
        if (iy < 0.0f) oy = iy + (float)ScreenHeight();
        if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
    }

    virtual void Draw(int x, int y, short c = 0x2588, short col = 0x000F) {
        float fx, fy;
        WrapCoordinates(x, y, fx, fy);
        olcConsoleGameEngine::Draw(fx, fy, c, col);
    }

    void ResetGame() {
        asteroids.clear();
        bullets.clear();
        asteroids.push_back({ 20.0f, 20.0f, 8.0f, -6.0f, (int)16, 0.0f });
        asteroids.push_back({ 100.0f, 20.0f, -5.0f, 3.0f, (int)16, 0.0f });

        // Initialise Player Position
        player.x = ScreenWidth() / 2.0f;
        player.y = ScreenHeight() / 2.0f;
        player.vx = 0.0f;
        player.vy = 0.0f;
        player.angle = 0.0f;

        bDead = false;
        nScore = 0;
    }
};


int main() {
    Asteroids game;
    game.ConstructConsole(160, 100, 8, 8);
    game.Start();
    return 0;
}
