#ifndef XMODELLER_H
#define XMODELLER_H

#include "X3DGlobal.h"
#include "XVector2D"
#include "XVector3D"
#include "XList"
#include "XGeometry.h"
#include "XSize"
#include "XTransform.h"

template <typename T> class XAbstractCurve;

class EKS3D_EXPORT XModeller
    {
public:
    XModeller( XGeometry * );
    ~XModeller( );

    void flush( );

    // Fixed Functionality GL Emulation
    enum Type { None, Quads, Triangles, Lines, Points };
    void begin( Type = Triangles );
    void end( );

    void vertex( XVector3D );
    inline void vertex( xReal, xReal, xReal );
    void normal( XVector3D );
    inline void normal( xReal, xReal, xReal );
    void texture( XVector2D );
    inline void texture( xReal, xReal );
    void colour( XVector4D );
    inline void colour( xReal, xReal, xReal, xReal = 1.0 );

    void setNormalsAutomatic( bool=true );
    bool normalsAutomatic( ) const;

    // Draw Functions
    void drawGeometry( const XGeometry & );
    void drawGeometry( XList <XVector3D> positions, const XGeometry & );

    void drawWireCube( const XCuboid &cube );

    void drawCube( XVector3D horizontal=XVector3D(1,0,0), XVector3D vertical=XVector3D(0,1,0), XVector3D depth=XVector3D(0,0,1), float tX=0.0, float tY=0.0 );
    void drawQuad( XVector3D horizontal, XVector3D vertical );
    void drawLocator( XSize size=XSize(1,1,1), XVector3D center=XVector3D() );

    void drawCurve( const XAbstractCurve <XVector3D> &, int segments );

    void setTransform( XTransform );
    XTransform transform( ) const;

    void save();
    void restore();

private:
    inline XVector3D transformPoint( XVector3D );
    inline XGeometry::Vertex3D transformPoint( XGeometry::Vertex3D );
    inline XVector <XVector3D> transformPoints( const XVector <XVector3D> & );
    inline XVector <XGeometry::Vertex3D> transformPoints( const XVector <XGeometry::Vertex3D> & );

    inline XVector3D transformNormal( XVector3D );
    inline XGeometry::Vertex3D transformNormal( XGeometry::Vertex3D );
    inline XVector <XVector3D> transformNormals( const XVector <XVector3D> & );
    inline XVector <XGeometry::Vertex3D> transformNormals( const XVector <XGeometry::Vertex3D> & );

    XGeometry *_geo;
    XList <unsigned int> _triIndices;
    XList <unsigned int> _linIndices;
    XList <unsigned int> _poiIndices;
    XList <XGeometry::Vertex3D> _vertex;
    XList <XGeometry::Vertex2D> _texture;
    XList <XGeometry::Vertex3D> _normals;
    XList <XGeometry::Vertex4D> _colours;

    struct State
        {
        State() : type( None ), normalsAutomatic( false ) { }
        XGeometry::Vertex3D normal;
        XGeometry::Vertex2D texture;
        XGeometry::Vertex4D colour;
        Type type;
        bool normalsAutomatic;
        };
    QList <State> _states;

    XTransform _transform;
    int _quadCount;
    };

void XModeller::vertex( xReal x, xReal y, xReal z )
    { vertex( XVector3D(x,y,z) ); }

void XModeller::normal( xReal x, xReal y, xReal z )
    { normal( XVector3D(x,y,z) ); }

void XModeller::texture( xReal x, xReal y )
    { texture( XVector2D(x,y) ); }

void XModeller::colour( xReal x, xReal y, xReal z, xReal w )
    { colour( XVector4D(x,y,z,w) ); }

#endif // XMODELLER_H