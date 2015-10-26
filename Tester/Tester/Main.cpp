#include <iostream>
#include <ogr_spatialref.h>
#include "ogrsf_frmts.h"
#include "gdal_alg.h"
#include "gdal_alg_priv.h"
#include "gdal_priv.h"

using namespace std;

int main()
{

	OGRRegisterAll();

	//Open a polygon shapefile and a polyline shapefile
	OGRDataSource       *pointDS = OGRSFDriverRegistrar::Open("E:\\GitClones\\Tester\\polyLine.shp", FALSE);
	OGRDataSource       *polyDS = OGRSFDriverRegistrar::Open("E:\\GitClones\\Tester\\poly.shp", FALSE);

	if (pointDS == NULL)
	{
		printf("Open failed for line.\n");
		exit(1);
	}
	if (polyDS == NULL)
	{
		printf("Open failed for poly.\n");
		exit(1);
	}

	//Extract the layers
	OGRLayer  *pointLayer;
	OGRLayer  *polyLayer;
	pointLayer = pointDS->GetLayer(0);
	polyLayer = polyDS->GetLayer(0);

	//Extract the features
	OGRFeature *pointFeature;
	OGRFeature *polyFeature;
	pointFeature = pointLayer->GetNextFeature();
	polyFeature = polyLayer->GetNextFeature();

	//Get the geometry of the features
	OGRGeometry *poGeometryPoint;
	OGRGeometry *poGeometryPoly;
	poGeometryPoint = pointFeature->GetGeometryRef();
	poGeometryPoly = polyFeature->GetGeometryRef();

	//Example: find the intersection of the polygon and a line... result is a line segment
	//Prints geometry type and then prints the points defining the line
	OGRGeometry *poGeometryIntersection = poGeometryPoly->Intersection(poGeometryPoint);
	cout << poGeometryIntersection->getGeometryType() << endl;
	OGRLineString *polyline = (OGRLineString*)poGeometryIntersection;
	for (int i = 0; i < polyline->getNumPoints(); i++)
	{
		cout << "point " << polyline->getX(i) << " " << polyline->getY(i) << endl;
	}
	delete poGeometryIntersection;

	//Translate polygon:
	OGRLinearRing *exteriorRing;
	OGRLinearRing *interiorRing;

	vector<pair<double,double>> newPointsExterior;
	vector<pair<double, double>> newPointsInterior;
	OGRPolygon * poPolygon = (OGRPolygon *)poGeometryPoly;
	exteriorRing = poPolygon->getExteriorRing();
	for (int i = 0; i < exteriorRing->getNumPoints(); i++)
	{
		pair<double, double> point;
		point.first = exteriorRing->getX(i)+25;
		point.second = exteriorRing->getY(i)+25;
		newPointsExterior.push_back(point);
	}
	for (int j = 0; j < poPolygon->getNumInteriorRings(); j++)
	{
		interiorRing = poPolygon->getInteriorRing(j);
		for (int i = 0; i < interiorRing->getNumPoints(); i++)
		{
			pair<double, double> point;
			point.first = interiorRing->getX(i) + 25;
			point.second = interiorRing->getY(i) + 25;
			newPointsInterior.push_back(point);
		}
	}


	//Now create a datasource:
	const char *pszDriverName = "ESRI Shapefile";
	OGRSFDriver *poDriver;
	
	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
		pszDriverName);

	OGRDataSource *poDS = poDriver->CreateDataSource("E:\\GitClones\\Tester\\polyShifted", NULL);
	if (poDS == NULL)
	{
		printf("Creation of output file failed.\n");
		exit(1);
	}

	//Create Layer
	OGRLayer *poLayer;

	poLayer = poDS->CreateLayer("polygon", NULL, wkbPolygon, NULL);
	if (poLayer == NULL)
	{
		printf("Layer creation failed.\n");
		exit(1);
	}
	
	//Create a polygon from the previous polygon
	OGRPolygon *newPoly = new OGRPolygon();
	OGRLinearRing *exteriorRing2=new OGRLinearRing();
	for (int i = 0; i < newPointsExterior.size(); i++)
	{
		exteriorRing2->addPoint(newPointsExterior[i].first, newPointsExterior[i].second);
	}
	newPoly->addRing(exteriorRing2);

	//
	OGRFeature *poFeature = polyFeature->Clone();
	poFeature->SetGeometry(newPoly);
	poLayer->CreateFeature(poFeature);
	
	OGRDataSource::DestroyDataSource(poDS);

	return 0;
}

