#ifndef XGLRENDERER_H
#define XGLRENDERER_H

#include "XRenderer.h"

class QGLContext;
class XGLShader;
class XGLFramebuffer;

class EKS3D_EXPORT XGLRenderer : public XRenderer
    {
public:
    XGLRenderer( bool multiSample=false );

    QGLContext *context();
    const QGLContext *context() const;

    void intialise();

    virtual XAbstractShader *getShader( );
    virtual XAbstractGeometry *getGeometry( XGeometry::BufferType );
    virtual XAbstractTexture *getTexture();
    virtual XAbstractFramebuffer *getFramebuffer( int options, int cf, int df, int width, int heightg );

    virtual void destroyShader( XAbstractShader * );
    virtual void destroyGeometry( XAbstractGeometry * );
    virtual void destroyTexture( XAbstractTexture * );
    virtual void destroyFramebuffer( XAbstractFramebuffer * );

    virtual void pushTransform( const XTransform & );
    virtual void popTransform( );

    virtual void clear();

    virtual void enableRenderFlag( RenderFlags );
    virtual void disableRenderFlag( RenderFlags );
    virtual int enabledFeatures( ) const;

    virtual void setViewportSize( QSize );
    virtual void setProjectionTransform( const XTransform & );

    virtual void setShader( const XShader * );

    virtual void drawGeometry( const XGeometry & );

    virtual void setFramebuffer( const XFramebuffer * );


    QSize viewportSize();
private:
    QGLContext *_context;
    XGLShader *_currentShader;
    QSize _size;
    XGLFramebuffer *_currentFramebuffer;
    int _features;
    };

#endif // XGLRENDERER_H
