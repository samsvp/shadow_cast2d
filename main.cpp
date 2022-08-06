#include "raylib.h"
#include "raymath.h"

#include <tuple>
#include <math.h>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>


const int NORTH = 0;
const int SOUTH = 1;
const int EAST = 2;
const int WEST = 3;


struct Edge
{
	float sx, sy; // Start coordinate
	float ex, ey; // End coordinate
};


struct Cell
{
	int edge_id[4];
	bool edge_exist[4];
	bool exist = false;
};


struct Point
{
    float angle;
    float x;
    float y;
};


struct Timing
{
    float dt;
    float lastTime;
    float currentTime;
} timing;



struct World
{
    const int worldScreenWidth = 800;
    const int worldScreenHeight = 600;
    
    const int nWorldWidth = 80;
    const int nWorldHeight = 60;

    const float fBlockWidth = (float)worldScreenWidth / nWorldWidth;

    Vector2 lightPos;

    std::vector<Cell> world;

    std::vector<Edge> edges;
    std::vector<Point> points;

    World() : world(nWorldWidth * nWorldHeight)
    {
        this->lightPos = {worldScreenWidth / 2.0f, worldScreenHeight / 2.0f};
    }
} mworld;



void drawWorld(float fBlockWidth, float rectHeight, Vector2 lightPos, bool drawLight)
{
    
    if (!mworld.points.empty() && drawLight)
    {
        // draw light
        for (size_t i = 0; i < mworld.points.size() - 1; i++)
        {
            // raylib wants the triangle points in counter clockwise order
            DrawTriangle(lightPos,
                {mworld.points[i].x,
				 mworld.points[i].y}, 

                {mworld.points[i + 1].x,
				 mworld.points[i + 1].y}, WHITE);

            DrawTriangle(lightPos, 
                {mworld.points[i + 1].x,
				 mworld.points[i + 1].y},

                {mworld.points[i].x,
				 mworld.points[i].y}, WHITE);
        }

        
        // last point
        DrawTriangle(lightPos, 
            {mworld.points[0].x,
			 mworld.points[0].y}, 
            {mworld.points[mworld.points.size() - 1].x,
			 mworld.points[mworld.points.size() - 1].y}, WHITE);
        DrawTriangle(lightPos, 
            {mworld.points[mworld.points.size() - 1].x,
			 mworld.points[mworld.points.size() - 1].y}, 
            {mworld.points[0].x,
			 mworld.points[0].y}, WHITE);
    }


    // Draw Blocks from TileMap
    for (int x = 0; x < mworld.nWorldWidth; x++)
        for (int y = 0; y < mworld.nWorldHeight; y++)
        {
            if (mworld.world[y * mworld.nWorldWidth + x].exist)
                DrawRectangle(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, BLUE);
        }

    // Draw Edges from PolyMap
    for (auto &e : mworld.edges)
    {
        DrawLine(e.sx, e.sy, e.ex, e.ey, GREEN);
        DrawCircle(e.sx, e.sy, 3, RED);
        DrawCircle(e.ex, e.ey, 3, RED);
    }
    
}


void ConvertTileMapToPolyMap(int w, int h, float fBlockWidth, int pitch)
{
    // Clear "PolyMap"
    mworld.edges.clear();

    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            for (int j = 0; j < 4; j++)
            {
                mworld.world[y * pitch + x].edge_exist[j] = false;
                mworld.world[y * pitch + x].edge_id[j] = 0;
            }

    // Iterate through region from top left to bottom right
    for (int x = 1; x < w - 1; x++)
    for (int y = 1; y < h - 1; y++)
    {
        // Create some convenient indices
        int i = y * pitch + x;			// This
        int n = (y - 1) * pitch + x;		// Northern Neighbour
        int s = (y + 1) * pitch + x;		// Southern Neighbour
        int w = y * pitch + (x - 1);	// Western Neighbour
        int e = y * pitch + (x + 1);	// Eastern Neighbour

        // If this cell exists, check if it needs edges
        if (!mworld.world[i].exist) continue;
        
        // If this cell has no western neighbour, it needs a western edge
        if (!mworld.world[w].exist)
        {
            // It can either extend it from its northern neighbour if they have
            // one, or It can start a new one.
            if (mworld.world[n].edge_exist[WEST])
            {
                // Northern neighbour has a western edge, so grow it downwards
                mworld.edges[mworld.world[n].edge_id[WEST]].ey += fBlockWidth;
                mworld.world[i].edge_id[WEST] = mworld.world[n].edge_id[WEST];
                mworld.world[i].edge_exist[WEST] = true;
            }
            else
            {
                // Northern neighbour does not have one, so create one
                Edge edge;
                edge.sx = x * fBlockWidth; edge.sy = y * fBlockWidth;
                edge.ex = edge.sx; edge.ey = edge.sy + fBlockWidth;

                // Add edge to Polygon Pool
                int edge_id = mworld.edges.size();
                mworld.edges.push_back(edge);

                // Update tile information with edge information
                mworld.world[i].edge_id[WEST] = edge_id;
                mworld.world[i].edge_exist[WEST] = true;
            }
        }

        // If this cell dont have an eastern neighbour, It needs a eastern edge
        if (!mworld.world[e].exist)
        {
            // It can either extend it from its northern neighbour if they have
            // one, or It can start a new one.
            if (mworld.world[n].edge_exist[EAST])
            {
                // Northern neighbour has one, so grow it downwards
                mworld.edges[mworld.world[n].edge_id[EAST]].ey += fBlockWidth;
                mworld.world[i].edge_id[EAST] = mworld.world[n].edge_id[EAST];
                mworld.world[i].edge_exist[EAST] = true;
            }
            else
            {
                // Northern neighbour does not have one, so create one
                Edge edge;
                edge.sx = (x + 1) * fBlockWidth; edge.sy = y * fBlockWidth;
                edge.ex = edge.sx; edge.ey = edge.sy + fBlockWidth;

                // Add edge to Polygon Pool
                int edge_id = mworld.edges.size();
                mworld.edges.push_back(edge);

                // Update tile information with edge information
                mworld.world[i].edge_id[EAST] = edge_id;
                mworld.world[i].edge_exist[EAST] = true;
            }
        }

        // If this cell doesnt have a northern neighbour, It needs a northern edge
        if (!mworld.world[n].exist)
        {
            // It can either extend it from its western neighbour if they have
            // one, or It can start a new one.
            if (mworld.world[w].edge_exist[NORTH])
            {
                // Western neighbour has one, so grow it eastwards
                mworld.edges[mworld.world[w].edge_id[NORTH]].ex += fBlockWidth;
                mworld.world[i].edge_id[NORTH] = mworld.world[w].edge_id[NORTH];
                mworld.world[i].edge_exist[NORTH] = true;
            }
            else
            {
                // Western neighbour does not have one, so create one
                Edge edge;
                edge.sx = x * fBlockWidth; edge.sy = y * fBlockWidth;
                edge.ex = edge.sx + fBlockWidth; edge.ey = edge.sy;

                // Add edge to Polygon Pool
                int edge_id = mworld.edges.size();
                mworld.edges.push_back(edge);

                // Update tile information with edge information
                mworld.world[i].edge_id[NORTH] = edge_id;
                mworld.world[i].edge_exist[NORTH] = true;
            }
        }

        // If this cell doesnt have a southern neighbour, It needs a southern edge
        if (!mworld.world[s].exist)
        {
            // It can either extend it from its western neighbour if they have
            // one, or It can start a new one.
            if (mworld.world[w].edge_exist[SOUTH])
            {
                // Western neighbour has one, so grow it eastwards
                mworld.edges[mworld.world[w].edge_id[SOUTH]].ex += fBlockWidth;
                mworld.world[i].edge_id[SOUTH] = mworld.world[w].edge_id[SOUTH];
                mworld.world[i].edge_exist[SOUTH] = true;
            }
            else
            {
                // Western neighbour does not have one, so I need to create one
                Edge edge;
                edge.sx = x * fBlockWidth; edge.sy = (y + 1) * fBlockWidth;
                edge.ex = edge.sx + fBlockWidth; edge.ey = edge.sy;

                // Add edge to Polygon Pool
                int edge_id = mworld.edges.size();
                mworld.edges.push_back(edge);

                // Update tile information with edge information
                mworld.world[i].edge_id[SOUTH] = edge_id;
                mworld.world[i].edge_exist[SOUTH] = true;
            }
        }

    }

}

void CalculateVisibilityPolygon(float ox, float oy, float radius)
{
    // Get rid of existing polygon
    mworld.points.clear();

    // For each edge in PolyMap
    for (auto &e1 : mworld.edges)
    {
        // Take the start point, then the end point (we could use a pool of
        // non-duplicated points here, it would be more optimal)
        for (int i = 0; i < 2; i++)
        {
            float rdx, rdy;
            rdx = (i == 0 ? e1.sx : e1.ex) - ox;
            rdy = (i == 0 ? e1.sy : e1.ey) - oy;

            float base_ang = atan2f(rdy, rdx);

            float ang = 0;
            // For each point, cast 3 rays, 1 directly at point
            // and 1 a little bit either side
            for (int j = 0; j < 3; j++)
            {
                if (j == 0)	ang = base_ang - 0.0001f;
                if (j == 1)	ang = base_ang;
                if (j == 2)	ang = base_ang + 0.0001f;

                // Create ray along angle for required distance
                rdx = radius * cosf(ang);
                rdy = radius * sinf(ang);

                float min_t1 = INFINITY;
                Point min_point;
                bool is_valid = false;

                // Check for ray intersection with all edges
                for (auto &e2 : mworld.edges)
                {
                    // Create line segment vector
                    float sdx = e2.ex - e2.sx;
                    float sdy = e2.ey - e2.sy;

                    if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
                    {
                        // t2 is normalised distance from line segment start to line segment end of intersect point
                        float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx);
                        // t1 is normalised distance from source along ray to ray length of intersect point
                        float t1 = (e2.sx + sdx * t2 - ox) / rdx;

                        // If intersect point exists along ray, and along line 
                        // segment then intersect point is valid
                        if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
                        {
                            // Check if this intersect point is closest to source. If
                            // it is, then store this point and reject others
                            if (t1 < min_t1)
                            {
                                min_t1 = t1;
                                min_point.x = ox + rdx * t1;
                                min_point.y = oy + rdy * t1;
                                min_point.angle = atan2f(min_point.y - oy, min_point.x - ox);
                                is_valid = true;
                            }
                        }
                    }
                }

                // Add intersection point to visibility polygon perimeter
                if(is_valid) mworld.points.push_back(min_point);
            }
        }
    }

    // Sort perimeter points by angle from source. This will allow
    // us to draw a triangle fan.
    std::sort(mworld.points.begin(), mworld.points.end(),
        [&](const Point &p1, const Point &p2) { return p1.angle < p2.angle; });

}


void start()
{
    // Add a boundary to the world
    for (int x = 1; x < (mworld.nWorldWidth - 1); x++)
    {
        mworld.world[1 * mworld.nWorldWidth + x].exist = true;
        mworld.world[(mworld.nWorldHeight - 2) * mworld.nWorldWidth + x].exist = true;
    }

    for (int x = 1; x < (mworld.nWorldHeight - 1); x++)
    {
        mworld.world[x * mworld.nWorldWidth + 1].exist = true;
        mworld.world[x * mworld.nWorldWidth + (mworld.nWorldWidth - 2)].exist = true;
    }

    ConvertTileMapToPolyMap(mworld.nWorldWidth, mworld.nWorldHeight, 
        mworld.fBlockWidth, mworld.nWorldWidth);
    CalculateVisibilityPolygon(mworld.lightPos.x, mworld.lightPos.y, 1000.0f);
}


int main()
{
    InitWindow(mworld.worldScreenWidth, mworld.worldScreenHeight, "Shadow Cast");

    timing.lastTime = GetTime();

    // set simulation initial state
    start();
    float speed = 200;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
		float fSourceX = GetMouseX();
		float fSourceY = GetMouseY();

        // Update timing
        timing.currentTime = GetTime();
        timing.dt = timing.currentTime - timing.lastTime;
        timing.lastTime = timing.currentTime;

        // Movement
        Vector2 lastLightPos = mworld.lightPos;
        if (IsKeyDown(KEY_D)) mworld.lightPos.x += speed * timing.dt;
        if (IsKeyDown(KEY_A)) mworld.lightPos.x -= speed * timing.dt;
        if (IsKeyDown(KEY_W)) mworld.lightPos.y -= speed * timing.dt;
        if (IsKeyDown(KEY_S)) mworld.lightPos.y += speed * timing.dt;
        
        if (lastLightPos.x != mworld.lightPos.x || lastLightPos.y != mworld.lightPos.y)
            CalculateVisibilityPolygon(mworld.lightPos.x, mworld.lightPos.y, 1000.0f);

        // toggle new tile
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            int x = GetMouseX() / mworld.fBlockWidth;
            int y = GetMouseY() / mworld.fBlockWidth;
            int i = y * mworld.nWorldWidth + x;
            mworld.world[i].exist = !mworld.world[i].exist;

            ConvertTileMapToPolyMap(mworld.nWorldWidth, mworld.nWorldHeight, 
                mworld.fBlockWidth, mworld.nWorldWidth);
            CalculateVisibilityPolygon(mworld.lightPos.x, mworld.lightPos.y, 1000.0f);
        }

        bool drawLight = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            
            drawWorld(mworld.fBlockWidth, mworld.fBlockWidth, mworld.lightPos, drawLight);
            DrawCircleV(mworld.lightPos, 10.0f, RED);

            DrawText((std::string("FPS ") + std::to_string(1.0 / timing.dt)).c_str(),
                10, 5, 20, WHITE);
                
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}