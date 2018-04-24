#pragma once

#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include "edge_heap.h"
#include "geo.h"
#include "csv.h"
#include "mymagick.h"

using namespace std;

// Might use as a lookup from city name to ID.
// And to filter duplicate cities.
typedef map<string, int> strMapInt;

typedef map<int, int> intint;

struct myMinMax
{
    double xMin;
    double xMax;
    double yMin;
    double yMax;
    myMinMax()
    {
        double minX = pow(2.0, 20.0);
        double maxX = 0;
        double minY = pow(2.0, 20.0);
        double maxY = 0;
    }

    void minX(double x)
    {
        if (x < xMin)
        {
            xMin = x;
        }
    }
    void maxX(double x)
    {
        if (x > xMax)
        {
            xMax = x;
        }
    }
    void minY(double y)
    {
        if (y < yMin)
        {
            yMin = y;
        }
    }
    void maxY(double y)
    {
        if (y > yMax)
        {
            yMax = y;
        }
    }
    void checkX(double x)
    {
        maxX(x);
        minX(x);
    }
    void checkY(double y)
    {
        maxY(y);
        minY(y);
    }
    friend ostream &operator<<(ostream &output, const myMinMax &mm)
    {
        output << "[min:(" << mm.xMin << "," << mm.yMin << "),max:(" << mm.xMax << "," << mm.yMax << ")]";
        return output;
    }
};

/**
 * vertex - represents a vertex in a graph.
 */
struct vertex
{
    int ID;
    string city;
    string state;
    latlon loc;
    point p;
    vector<edge> E;
    bool visited;

    point t;

    /**
     * vertex<< - overload cout for vertex
     * Params:
     *     const vertex v     - vertex to print
     * Returns 
     *     formatted output for a vertex
     */
    vertex(int id, string c, string s, latlon ll = latlon())
    {
        ID = id;
        city = c;
        state = s;
        loc = ll;
        p.setXY(lon2x(ll.lon), lat2y(ll.lat));
        visited = false;
    }

    void update(latlon ll)
    {
        loc = ll;
        p.setXY(lon2x(ll.lon), lat2y(ll.lat));
    }

    bool Neighbors(int id)
    {
        vector<edge>::iterator eit;
        if (E.size() ==  0)
        {
            return false;

        }else{
            for (eit = E.begin(); eit != E.end(); eit++)
            {
                if((*eit).toID == id){
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * operator<< - overload cout for vertex
     * Params:
     *     const vertex v     - vertex to print
     * Returns 
     *     formatted output for a vertex
     */
    friend ostream &operator<<(ostream &output, const vertex &v)
    {
        output << "(ID:" << v.ID << ", " << v.city << ", " << v.state << ", LatLon: " << v.loc << " Point: " << v.p << ", Edges:" << v.E.size() << ")";
        return output;
    }
};

/**
 * graph - set of vertices and edges
 * 
 * Methods (private):
 *     vertex* createVertex(string city,string state)
 * Methods (public):
 *     graph()
 */
class graph
{
  public:
    int id;                      // id counter for new vertices
    int num_edges;               // edge count
    vector<vertex *> vertexList; // vector to hold vertices
    strMapInt cityLookup;
    llBox box; // bounding box of coords

    /**
     * private: createVertex - returns a new vertex with unique id.
     * Params:
     *     string city
     *     string state
     */
    vertex *createVertex(string city, string state, latlon ll)
    {
        return new vertex(id++, city, state, ll);
    }

    /**
     * graph - constructor
     */
    graph()
    {
        id = 0;
        num_edges = 0;
    }

    graph(const graph &G)
    {
        id = G.id;
        num_edges = G.num_edges;
        vertexList = G.vertexList;
        cityLookup = G.cityLookup;
    }

    /**
     * addVertex - adds a vertex to the graph
     * Params:
     *     string   city     - name of city
     *     string   state    - two letter abbr of state
     *     double   lat      - latitude 
     *     double   lon      - longitude 
     * Returns 
     *     void
     */
    int addVertex(string city, string state, double lat = 0.0, double lon = 0.0)
    {
        if (cityLookup.find(city) == cityLookup.end())
        {
            // Add the city as a key to the map.
            cityLookup[city] = 0;
        }
        else
        {
            return -1;
        }

        //create a bounding box of values to help with scaling for drawing.
        box.addLatLon(latlon(lat, lon));

        vertex *temp = createVertex(city, state, latlon(lat, lon));
        vertexList.push_back(temp);

        //update the value that city points to.
        cityLookup[city] = temp->ID;
        return temp->ID;
    }

    // void updateVLocation(int vid,latlon ll){
    //     box.addLatLon(latlon(ll.lat,ll.lon));
    //     vertexList[vid]->loc = ll;
    //     vertexList[vid]->p.setXY(lon2x(ll.lon),lat2y(ll.lat));
    // }

    vertex *getVertex(int id)
    {
        if (id >= 0 && id < vertexList.size())
        {
            return vertexList[id];
        }
        return NULL;
    }

    bool Connected()
    {
        vector<vertex *>::iterator i;

        for (i = vertexList.begin(); i != vertexList.end(); i++)
        {
            if ((*i)->E.size() == 0)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * addEdge - adds a relationship between two vertices to the graph
     * Params:
     *     int      fromID   - the ID of the vertex in which the edge is leaving
     *     int      toID     - ID of the receiving vertex
     *     double   weight   - weight of the edge if any 
     *     bool     directed - is the edge directed or not
     * Returns 
     *     void
     */
    void addEdge(int fromID, int toID, double weight = 0, bool directed = false, string color = "Black")
    {
        edge e1(fromID, toID, weight, color);
        vertexList[fromID]->E.push_back(e1);
        num_edges++;

        //cout<<"adding "<<fromID<<" to "<<toID<<endl;

        if (!directed)
        {
            edge e2(toID, fromID, weight, color);
            vertexList[toID]->E.push_back(e2);

            //cout<<"adding "<<toID<<" to "<<fromID<<endl;

            num_edges++;
        }
    }

    /**
     * addEdge - adds a relationship between two vertices to the graph
     * Params:
     *     string   fromCity   - the city of the vertex in which the edge is leaving
     *     string   toCity     - city of the receiving vertex
     *     double   weight     - weight of the edge if any 
     *     bool     directed   - is the edge directed or not
     * Returns:
     *     void
     */
    void addEdge(string fromCity, string toCity, double weight = 0, bool directed = false, string color = "Black")
    {
    }

    /**
     * printGraph - prints the graph out for debugging purposes
     * Params:
     *     void
     */
    void printGraph()
    {

        vector<vertex *>::iterator vit;
        vector<edge>::iterator eit;

        for (vit = vertexList.begin(); vit != vertexList.end(); vit++)
        {

            double brng = bearing((*(*vit)).loc, box.center);

            (*(*vit)).update(geo_destination((*(*vit)).loc, 100, brng));
            cout << (*(*vit)) << endl;

            if ((*vit)->E.size() > 0)
            {
                for (eit = (*vit)->E.begin(); eit != (*vit)->E.end(); eit++)
                {
                    cout << "\t" << (*eit) << endl;
                }
            }
        }
    }

    void magickGraph(int w, int h, string imageName)
    {
        drawGraph dg(w, h, "white");
        vector<vertex *>::iterator vit;
        vector<edge>::iterator eit;

        // For calculating new coords to stretch

        int x1;
        int y1;
        int x2;
        int y2;

        // For counting how much more
        int vsize = vertexList.size();
        int i = 0;
        double op = -10.0;
        double np = 0.0;

        int rx;
        int ry;

        int cx = ((box.c_p.x - box.minx) / (box.maxx - box.minx)) * w;
        int cy = h - (((box.c_p.y - box.miny) / (box.maxy - box.miny)) * h);

        //[LL(41.6306,-110.782),(20.9862,-85.5431)]
        //[XY(41.6306,249.218),(20.9862,274.457)]
        //[Center(31.3084,-98.1627),(31.3084,261.837)]

        for (vit = vertexList.begin(); vit != vertexList.end(); vit++)
        {

            x1 = (((*(*vit)).p.x - box.minx) / (box.maxx - box.minx)) * w;
            y1 = h - ((((*(*vit)).p.y - box.miny) / (box.maxy - box.miny)) * h);

            cout << (*(*vit)).city << endl;

            if ((*(*vit)).city == "Wichita Falls")
            {
                cout << "*******************" << endl;
                dg.setFillColor("Red");
                dg.setStrokeColor("Red");
                dg.drawCircleNode(x1, y1, 30, 30);
                rx = x1;
                ry = y1;
            }
            else
            {
                dg.setFontSize(20);
                dg.setFillColor("White");
                dg.setStrokeColor("Black");
                dg.drawRectangleNode(x1, y1, 150, 60, (*(*vit)).city);
            }

            if ((*vit)->E.size() > 0)
            {
                cout << (*vit)->E.size() << endl;
                for (eit = (*vit)->E.begin(); eit != (*vit)->E.end(); eit++)
                {
                    // cout<<(*vertexList[(*eit).toID]).p.x<<","<<(*vertexList[(*eit).toID]).p.y<<endl;
                    // cout<<(*vertexList[(*eit).toID]).p<<endl;
                    x2 = (((*vertexList[(*eit).toID]).p.x - box.minx) / (box.maxx - box.minx)) * w;
                    y2 = h - ((((*vertexList[(*eit).toID]).p.y - box.miny) / (box.maxy - box.miny)) * h);
                    if (x1 > 0 && y1 > 0 && x2 > 0 && y2 > 0)
                    {
                        //dg.setStrokeColor((*eit).color);
                        dg.drawLine(x1, y1, x2, y2,(*eit).color);
                        cout << x1 << "," << y1 << "," << x2 << "," << y2 << endl;
                    }

                    //cout<<"Adding line ...\n";
                }
            }

            i = i + 1;
            np = (double(i) / double(vsize)) * 100;
            if ((abs(np) - abs(op)) >= 10)
            {
                op = np;
                cout << np << " complete" << endl;
            }
        }

        //dg.drawLine(0,0,w,h);
        dg.setFillColor("Red");
        dg.setStrokeColor("Red");
        dg.drawCircleNode(rx, ry, 30, 30);
        //cout << rx << "," << ry << endl;
        //dg.drawLine(cx, cy, rx, ry);
        dg.writeImage(imageName);
        //cout << cx << "," << cy << endl;

        //cout << box << endl;
    }

    void expandGraph(int distance)
    {
        vector<vertex *>::iterator vit;

        latlon ll;
        latlon destination;
        latlon center = box.center;

        box.reset();
        for (vit = vertexList.begin(); vit != vertexList.end(); vit++)
        {
            ll = (*(*vit)).loc;

            double brng = bearing(center, ll);

            destination = geo_destination(ll, distance, brng);
            (*(*vit)).update(destination);
            box.addLatLon(destination);
        }
    }

    string mylower(string s)
    {
        for (int i = 0; i < s.length(); i++)
        {
            if (s[i] >= 'A' && s[i] <= 'Z')
            {
                s[i] += 32;
            }
        }
        return s;
    }

    string searchGraph(string c)
    {

        vector<vertex *>::iterator i;
        vector<edge>::iterator eit;

        for (i = vertexList.begin(); i != vertexList.end(); i++)
        {
            if (mylower((*i)->city) == mylower(c))
            {
                cout << *(*i) << endl;
                return (*i)->city;
            }
        }
        return "";
    }

    // find the three closest vertices and create edges between them.
    void createSpanningTree(string filter = "")
    {
        vector<vertex *>::iterator i;
        vector<vertex *>::iterator j;
        vector<edge>::iterator eit;
        edgeHeap E;
        edge *e;

        string colors[] = {"Black", "Blue", "Green", "Red", "Purple", "Orange", "Yellow", "Brown", "Pink"};

        double distance = 0;
        double minDistance = pow(2.0, 30.0);
        int closestID;
        int count = 0;
        int ecount = 0;

        while (!Connected())
        {

            // Outer loop through vertices
            for (i = vertexList.begin(); i != vertexList.end(); i++)
            {
                cout << "Connecting: " << (*i)->city << endl;
                // Inner loop through vertices finds closes neighbors
                for (j = vertexList.begin(); j != vertexList.end(); j++)
                {
                    if (!(*i)->Neighbors((*j)->ID))
                    {
                        distance = distanceEarth((*i)->loc.lat, (*i)->loc.lon, (*j)->loc.lat, (*j)->loc.lon);

                        if (distance > 0)
                        {
                            E.Insert(new edge((*i)->ID, (*j)->ID, distance));
                            ecount++;
                            if (ecount % 1000 == 0)
                            {
                                cout << "Edges: " << ecount << endl;
                            }
                        }
                    }
                }

                e = E.Extract();
                cout << *e << endl;

                addEdge(e->fromID, e->toID, e->weight, false, colors[count%9]);


                E.ClearHeap();
            }
            count++;
        }
    }

    void printVids()
    {
        vector<vertex *>::iterator vit;
        vector<edge>::iterator eit;

        for (vit = vertexList.begin(); vit != vertexList.end(); vit++)
        {
            cout << (*vit)->ID << endl;
        }
    }

    string graphViz(bool directed = true, int scale = 0)
    {
        vector<vertex *>::iterator vit;
        vector<edge>::iterator eit;

        // [label="hi", weight=100];

        string viz = "";
        string labels = "";
        string conns = "";
        int weight = 0;
        string arrow = "";
        int x;
        int y;

        if (directed)
        {
            viz = "digraph G {\n";
            arrow = "->";
        }
        else
        {
            viz = "graph G {\n";
            arrow = "--";
        }

        for (vit = vertexList.begin(); vit != vertexList.end(); vit++)
        {
            if ((*vit)->E.size() > 0)
            {
                x = (*vit)->p.x;
                y = (*vit)->p.y;

                labels += "\t" + to_string((*vit)->ID) + " [\n\t\tlabel=\"" + (*vit)->city + ", " + (*vit)->state + "\" \n\t\tpos = \"" + to_string(x) + "," + to_string(y) + "!\"\n\t]\n";

                for (eit = (*vit)->E.begin(); eit != (*vit)->E.end(); eit++)
                {
                    x = vertexList[eit->toID]->p.x;
                    y = vertexList[eit->toID]->p.y;
                    labels += "\t" + to_string(eit->toID) + " [\n\t\tlabel=\"" + vertexList[eit->toID]->city + ", " + vertexList[eit->toID]->state + "\"]\n\t\tpos = \"" + to_string(x) + "," + to_string(y) + "!\"\n\t]\n";
                    weight = eit->weight;
                    conns += "\t" + to_string((*vit)->ID) + arrow + to_string(eit->toID) + " [weight=" + to_string(weight) + ", label=" + to_string(weight) + "]\n";
                }
            }
            else
            {
                x = (*vit)->p.x;
                y = (*vit)->p.y;

                labels += "\t" + to_string((*vit)->ID) + " [\n\t\tlabel=\"" + (*vit)->city + ", " + (*vit)->state + "\" \n\t\tpos = \"" + to_string(x) + "," + to_string(y) + "!\"\n\t]\n";
            }
        }

        viz += labels;
        viz += conns;
        viz += "}\n";

        return viz;
    }

    /**
     * maxID - returns the max id assigned to any vertex
     * Params:
     *     void
     * Returns:
     *     int
     */
    int maxID()
    {
        return id;
    }

    /**
     * graphSize - returns the number of vertices and edges
     * Params:
     *     void
     * Returns:
     *     int
     */
    int *graphSize()
    {
        int *vals = new int[2];
        vals[0] = vertexList.size();
        vals[1] = num_edges;
        return vals;
    }

    /**
     * operator= - overload assignment for Graph
     * NOT DONE
     * Params:
     *     const latlon ll     - lat lon to assign
     * Returns 
     *     reference to assignment for chaining (e.g. a = b = c )
     */
    graph &operator=(const graph &g)
    {
        // do the copy
        vertexList = g.vertexList;
        id = g.id;

        // return the existing object so we can chain this operator
        return *this;
    }
};