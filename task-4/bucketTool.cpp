#include "bucketTool.h"
#include "canvas.h" 
#include <QDebug>
#include <QPainter>
#include <QQueue>
#include <vector>

BucketTool::BucketTool(Canvas* canvasInstance)
    : canvas(canvasInstance),
      currentFillColor(Qt::black), 
      activeFillTexture(nullptr),
      useTextureFill(false),
      performFill(false)
{
}

BucketTool::~BucketTool() {
}

Figure* BucketTool::onMousePress(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && canvas) {
        startPoint = event->pos();
        
        QPixmap pixmap = canvas->grab(QRect(QPoint(0,0), canvas->size()));
        initialCanvasImage = pixmap.toImage();

        if (startPoint.x() >= 0 && startPoint.x() < initialCanvasImage.width() &&
            startPoint.y() >= 0 && startPoint.y() < initialCanvasImage.height()) {
            targetColor = initialCanvasImage.pixelColor(startPoint);
            performFill = true;
            canvas->update(); 
        } else {
            performFill = false; 
        }
    }
    return nullptr; 
}

bool BucketTool::onMouseMove(QMouseEvent * /*event*/) {
    return false; 
}

std::string BucketTool::name() const {
    return "Bucket Tool";
}

void BucketTool::draw(QPainter &painter) {
    if (!performFill || !canvas || initialCanvasImage.isNull()) {
        performFill = false;
        return;
    }

    QImage workingImage = initialCanvasImage; 
    int canvasWidth = workingImage.width();
    int canvasHeight = workingImage.height();

    std::queue<QPoint> q;

    
    if (startPoint.x() < 0 || startPoint.x() >= canvasWidth ||
        startPoint.y() < 0 || startPoint.y() >= canvasHeight ||
        initialCanvasImage.pixelColor(startPoint) != targetColor) {
        performFill = false;
        return;
    }

    q.push(startPoint);

    std::vector<std::vector<bool>> visited(canvasWidth, std::vector<bool>(canvasHeight, false));

    
    
    
    

    while (!q.empty()) {
        QPoint seed = q.front();
        q.pop();

        if (seed.x() < 0 || seed.x() >= canvasWidth || seed.y() < 0 || seed.y() >= canvasHeight ||
            visited[seed.x()][seed.y()] || initialCanvasImage.pixelColor(seed) != targetColor) {
            continue;
        }

        int y = seed.y();
        int currentX = seed.x();

        // Find left extent
        int l = currentX;
        while (l >= 0 && initialCanvasImage.pixelColor(l, y) == targetColor && !visited[l][y]) {
            l--;
        }
        l++;

        // Find right extent
        int r = currentX;
        while (r < canvasWidth && initialCanvasImage.pixelColor(r, y) == targetColor && !visited[r][y]) {
            r++;
        }
        r--;

        
        for (int x = l; x <= r; ++x) {
            if (useTextureFill && activeFillTexture && !currentFillTextureImage.isNull()) {
                int texWidth = currentFillTextureImage.width();
                int texHeight = currentFillTextureImage.height();
                int texX = x % texWidth;
                int texY = y % texHeight;
                if (texX < 0) texX += texWidth;
                if (texY < 0) texY += texHeight;
                workingImage.setPixelColor(x, y, currentFillTextureImage.pixelColor(texX, texY));
            } else {
                workingImage.setPixelColor(x, y, currentFillColor);
            }
            visited[x][y] = true;

            // Check pixel above
            if (y > 0 && initialCanvasImage.pixelColor(x, y - 1) == targetColor && !visited[x][y - 1]) {
                q.push(QPoint(x, y - 1));
            }
            // Check pixel below
            if (y < canvasHeight - 1 && initialCanvasImage.pixelColor(x, y + 1) == targetColor && !visited[x][y + 1]) {
                q.push(QPoint(x, y + 1));
            }
        }
    }

    painter.drawImage(QRect(0,0, canvasWidth, canvasHeight), workingImage);
    performFill = false; 
}

void BucketTool::onMouseRelease(QMouseEvent * ) {
}

void BucketTool::setFillColor(const QColor& color) {
    currentFillColor = color;
    useTextureFill = false;
    activeFillTexture = nullptr; 
    currentFillTextureImage = QImage(); 
}

void BucketTool::setFillTexture(QPixmap* texture) {
    activeFillTexture = texture;
    if (activeFillTexture && !activeFillTexture->isNull()) {
        currentFillTextureImage = activeFillTexture->toImage();
        if (!currentFillTextureImage.isNull()) {
             useTextureFill = true;
        } else {
            
            useTextureFill = false;
            activeFillTexture = nullptr;
        }
    } else {
        useTextureFill = false;
        activeFillTexture = nullptr;
        currentFillTextureImage = QImage();
    }
}

QColor BucketTool::getFillColor() const {
    return currentFillColor;
}
