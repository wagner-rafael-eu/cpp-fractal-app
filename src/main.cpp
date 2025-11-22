#include <SFML/Graphics.hpp>
#include <complex>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>

constexpr double PI = 3.14159265358979323846;

// Function to calculate Mandelbrot set value for a point
int calculateMandelbrot(double x, double y, int maxIter) {
    std::complex<double> c(x, y);
    std::complex<double> z(0, 0);
    
    for (int i = 0; i < maxIter; ++i) {
        z = z * z + c;
        if (std::abs(z) > 2.0) {
            return i;
        }
    }
    return maxIter;
}

// Function to map a value from one range to another
double map(double value, double inMin, double inMax, double outMin, double outMax) {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

// Consistent helpers: pixel <-> complex conversions
inline double pixelToReal(int px, int width, double realMin, double realMax) {
    return map(static_cast<double>(px), 0.0, static_cast<double>(width - 1), realMin, realMax);
}
inline double pixelToImag(int py, int height, double imagMin, double imagMax) {
    // map pixel Y (0 = top) to complex imaginary where imagMax is top
    return map(static_cast<double>(py), 0.0, static_cast<double>(height - 1), imagMax, imagMin);
}
inline int realToPixel(double r, int width, double realMin, double realMax) {
    return static_cast<int>(map(r, realMin, realMax, 0.0, static_cast<double>(width - 1)));
}
inline int imagToPixel(double i, int height, double imagMin, double imagMax) {
    return static_cast<int>(map(i, imagMax, imagMin, 0.0, static_cast<double>(height - 1)));
}

// Draw a straight line between two pixel coordinates on an image
void drawLine(sf::Image &image, int x0, int y0, int x1, int y1, const sf::Color &col) {
    const int dx = std::abs(x1 - x0);
    const int dy = std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int x = x0, y = y0;
    while (true) {
        if (x >= 0 && x < (int)image.getSize().x && y >= 0 && y < (int)image.getSize().y)
            image.setPixel(x, y, col);
        if (x == x1 && y == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

// Mandelbrot renderer (was previous renderFractal)
void renderMandelbrot(sf::Image &image, int width, int height,
                      double realMin, double realMax,
                      double imagMin, double imagMax,
                      int maxIter) {
    int minX = width, minY = height, maxX = 0, maxY = 0;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            double realPart = pixelToReal(x, width, realMin, realMax);
            double imagPart = pixelToImag(y, height, imagMin, imagMax);

            int iterations = calculateMandelbrot(realPart, imagPart, maxIter);

            sf::Color color;
            if (iterations == maxIter) {
                color = sf::Color::Black;
            } else {
                uint8_t blue = static_cast<sf::Uint8>(map(iterations, 0, maxIter, 0, 255));
                color = sf::Color(0, 0, blue);
            }

            image.setPixel(x, y, color);
            if (color != sf::Color::Black) {
                minX = std::min(minX, x); maxX = std::max(maxX, x);
                minY = std::min(minY, y); maxY = std::max(maxY, y);
            }
        }
    }
    std::cout << "Mandelbrot drawn bounds px=["<<minX<<","<<maxX<<"] py=["<<minY<<","<<maxY<<"]\n";
}

// Sierpiński triangle using chaos game — compute in pixel space so zoom/center
// consistently affect the algorithm. This avoids subtle numeric mismatches
// between world<->pixel conversions when zooming and panning.
void renderSierpinski(sf::Image &image, int width, int height,
                      double realMin, double realMax,
                      double imagMin, double imagMax) {
    std::cout << "Sierpinski render bounds: ["<<realMin<<","<<realMax<<","<<imagMin<<","<<imagMax<<"]\n";
    // clear background
    image.create(width, height, sf::Color::Black);
    // compute triangle vertices in pixel coordinates by mapping world coords
    int v1x = realToPixel(realMin, width, realMin, realMax);
    int v1y = imagToPixel(imagMax, height, imagMin, imagMax);
    int v2x = realToPixel(realMax, width, realMin, realMax);
    int v2y = imagToPixel(imagMax, height, imagMin, imagMax);
    int v3x = realToPixel((realMin + realMax) / 2.0, width, realMin, realMax);
    int v3y = imagToPixel(imagMin, height, imagMin, imagMax);

    // start at centroid in pixel space
    double prx = (static_cast<double>(v1x) + v2x + v3x) / 3.0;
    double pry = (static_cast<double>(v1y) + v2y + v3y) / 3.0;

    const int iterations = 120000;
    int minX = width, minY = height, maxX = 0, maxY = 0;
    for (int i = 0; i < iterations; ++i) {
        int v = rand() % 3;
        if (v == 0) { prx = (prx + v1x) / 2.0; pry = (pry + v1y) / 2.0; }
        else if (v == 1) { prx = (prx + v2x) / 2.0; pry = (pry + v2y) / 2.0; }
        else { prx = (prx + v3x) / 2.0; pry = (pry + v3y) / 2.0; }

        int px = static_cast<int>(std::lround(prx));
        int py = static_cast<int>(std::lround(pry));
        if (px >= 0 && px < width && py >= 0 && py < height) {
            // Golden yellow color for Sierpinski (fractal 2)
            image.setPixel(px, py, sf::Color(255,215,0));
            minX = std::min(minX, px); maxX = std::max(maxX, px);
            minY = std::min(minY, py); maxY = std::max(maxY, py);
        }
    }
    std::cout << "Sierpinski drawn bounds px=["<<minX<<","<<maxX<<"] py=["<<minY<<","<<maxY<<"]\n";
}

// Koch curve renderer — operate in pixel space so view transforms (zoom/center)
// always take effect consistently. We recurse in pixel coordinates (double)
// and draw directly to the image without extra real<->pixel conversions.
void kochRecurse(std::vector<std::pair<double,double>> &pts, const std::pair<double,double> &a, const std::pair<double,double> &b, int depth) {
    if (depth == 0) {
        pts.push_back(a);
        return;
    }
    double ax = a.first, ay = a.second;
    double bx = b.first, by = b.second;
    double vx = bx - ax, vy = by - ay;
    std::pair<double,double> p1 = { ax + vx / 3.0, ay + vy / 3.0 };
    std::pair<double,double> p3 = { ax + vx * (2.0 / 3.0), ay + vy * (2.0 / 3.0) };
    double angle = atan2(vy, vx) - PI / 3.0;
    double len = std::hypot(vx, vy) / 3.0;
    std::pair<double,double> p2 = { p1.first + cos(angle) * len, p1.second + sin(angle) * len };
    kochRecurse(pts, a, p1, depth - 1);
    kochRecurse(pts, p1, p2, depth - 1);
    kochRecurse(pts, p2, p3, depth - 1);
    kochRecurse(pts, p3, b, depth - 1);
}

void renderKoch(sf::Image &image, int width, int height,
                double realMin, double realMax,
                double imagMin, double imagMax) {
    std::cout << "Koch render bounds: ["<<realMin<<","<<realMax<<","<<imagMin<<","<<imagMax<<"]\n";
    image.create(width, height, sf::Color::Black);
    // compute center imaginary coordinate and map endpoints into pixel coordinates
    double centerImag = (imagMin + imagMax) / 2.0;
    // endpoints in pixel space (map from world coordinates)
    std::pair<double,double> a_px = { static_cast<double>(realToPixel(realMin, width, realMin, realMax)), static_cast<double>(imagToPixel(centerImag, height, imagMin, imagMax)) };
    std::pair<double,double> b_px = { static_cast<double>(realToPixel(realMax, width, realMin, realMax)), static_cast<double>(imagToPixel(centerImag, height, imagMin, imagMax)) };
    int depth = 6;
    std::vector<std::pair<double,double>> pts;
    kochRecurse(pts, a_px, b_px, depth);
    pts.push_back(b_px);
    // draw lines using pixel coords already computed
    int minX = width, minY = height, maxX = 0, maxY = 0;
    for (size_t i = 1; i < pts.size(); ++i) {
        int x0 = static_cast<int>(std::lround(pts[i-1].first));
        int y0 = static_cast<int>(std::lround(pts[i-1].second));
        int x1 = static_cast<int>(std::lround(pts[i].first));
        int y1 = static_cast<int>(std::lround(pts[i].second));
        // Green color for Koch (fractal 3)
        drawLine(image, x0, y0, x1, y1, sf::Color(34,139,34));
        minX = std::min({minX, x0, x1}); maxX = std::max({maxX, x0, x1});
        minY = std::min({minY, y0, y1}); maxY = std::max({maxY, y0, y1});
    }
    std::cout << "Koch drawn bounds px=["<<minX<<","<<maxX<<"] py=["<<minY<<","<<maxY<<"]\n";
}

// Menger sponge 2D projection (Sierpinski carpet style)
void renderMenger(sf::Image &image, int width, int height,
                  double realMin, double realMax,
                  double imagMin, double imagMax) {
    std::cout << "Menger render bounds: ["<<realMin<<","<<realMax<<","<<imagMin<<","<<imagMax<<"]\n";
    // Use black background and draw the fractal in silver (light gray)
    image.create(width, height, sf::Color::Black);
    int minX = width, minY = height, maxX = 0, maxY = 0;
    // Use pixel coordinates normalized to 0..1 (left->right, bottom->top)
    // so zoom/center always transforms the result consistently.
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            double ux = static_cast<double>(x) / static_cast<double>(width - 1);
            double uy = 1.0 - static_cast<double>(y) / static_cast<double>(height - 1);
            bool hole = false;
            double tx = ux, ty = uy;
            for (int iter = 0; iter < 8; ++iter) {
                tx *= 3.0; ty *= 3.0;
                int ix = static_cast<int>(floor(tx));
                int iy = static_cast<int>(floor(ty));
                if (ix % 3 == 1 && iy % 3 == 1) { hole = true; break; }
                tx -= ix; ty -= iy;
            }
            if (hole) {
                // leave as background (black)
            } else {
                // Silver color for Menger (fractal 4)
                image.setPixel(x, y, sf::Color(192,192,192));
                minX = std::min(minX, x); maxX = std::max(maxX, x);
                minY = std::min(minY, y); maxY = std::max(maxY, y);
            }
        }
    }
    std::cout << "Menger drawn bounds px=["<<minX<<","<<maxX<<"] py=["<<minY<<","<<maxY<<"]\n";
}

// Dragon curve using L-system
std::string dragonLSystem(int depth) {
    std::string s = "FX";
    for (int i = 0; i < depth; ++i) {
        std::string next;
        for (char c : s) {
            if (c == 'X') next += "X+YF+";
            else if (c == 'Y') next += "-FX-Y";
            else next.push_back(c);
        }
        s.swap(next);
    }
    return s;
}

void renderDragon(sf::Image &image, int width, int height,
                  double realMin, double realMax,
                  double imagMin, double imagMax) {
    image.create(width, height, sf::Color::Black);
    int depth = 12;
    std::string prog = dragonLSystem(depth);
    double angle = 0.0;
    // Use a fixed reference world width so the dragon's geometry doesn't
    // cancel out when mapped to pixels. This makes the curve scale/translate
    // correctly as the view (realMin/realMax) changes.
    constexpr double REF_WORLD_WIDTH = 3.5; // initial world width (INIT_REAL_MAX - INIT_REAL_MIN)
    double step = (REF_WORLD_WIDTH) / static_cast<double>(width) * 2.0; // ~2 pixels per segment at reference scale
    double cx = (realMin + realMax) / 2.0;
    double cy = (imagMin + imagMax) / 2.0;
    double x = cx - (REF_WORLD_WIDTH) / 4.0;
    double y = cy;

    // First build the list of complex points for the curve, then map to pixels
    std::vector<std::pair<double,double>> points;
    points.reserve(prog.size());
    for (char c : prog) {
        if (c == 'F') {
            double nx = x + cos(angle) * step;
            double ny = y + sin(angle) * step;
            points.emplace_back(x, y);
            // advance
            x = nx; y = ny;
        } else if (c == '+') {
            angle += PI / 2.0;
        } else if (c == '-') {
            angle -= PI / 2.0;
        }
    }

    int minX = width, minY = height, maxX = 0, maxY = 0;
    // map points to pixels and draw lines between successive mapped points
    for (size_t i = 1; i < points.size(); ++i) {
        int x0 = realToPixel(points[i-1].first, width, realMin, realMax);
        int y0 = imagToPixel(points[i-1].second, height, imagMin, imagMax);
        int x1 = realToPixel(points[i].first, width, realMin, realMax);
        int y1 = imagToPixel(points[i].second, height, imagMin, imagMax);
        // color shades of red reminiscent of the China flag
        double t = static_cast<double>(i) / std::max<size_t>(1, points.size()-1);
        uint8_t r = static_cast<uint8_t>(std::min(255.0, 120.0 + 135.0 * t));
        sf::Color col(r, 20, 20);
        drawLine(image, x0, y0, x1, y1, col);
        minX = std::min({minX, x0, x1}); maxX = std::max({maxX, x0, x1});
        minY = std::min({minY, y0, y1}); maxY = std::max({maxY, y0, y1});
    }

    // Log diagnostic info: step and a few sample mapped pixels to see whether mapping varies
    std::cout << "Dragon step=" << step << " samples=";
    for (size_t i = 0; i < points.size() && i < 5; ++i) {
        int sx = realToPixel(points[i].first, width, realMin, realMax);
        int sy = imagToPixel(points[i].second, height, imagMin, imagMax);
        std::cout << "(" << sx << "," << sy << ")";
        if (i + 1 < points.size() && i + 1 < 5) std::cout << ",";
    }
    std::cout << "\n";

    std::cout << "Dragon drawn bounds px=["<<minX<<","<<maxX<<"] py=["<<minY<<","<<maxY<<"]\n";
}

// Master dispatcher
enum FractalType { MADELBROT = 1, SIERPINSKI = 2, KOCH = 3, MENGER = 4, DRAGON = 5 };

void renderCurrent(int fractal, sf::Image &image, int width, int height,
                   double realMin, double realMax, double imagMin, double imagMax, int maxIter) {
    std::cout << "renderCurrent called: fractal=" << fractal
              << " bounds=[" << realMin << "," << realMax << "," << imagMin << "," << imagMax << "]\n";
    switch (fractal) {
        case MADELBROT: renderMandelbrot(image, width, height, realMin, realMax, imagMin, imagMax, maxIter); break;
        case SIERPINSKI: renderSierpinski(image, width, height, realMin, realMax, imagMin, imagMax); break;
        case KOCH: renderKoch(image, width, height, realMin, realMax, imagMin, imagMax); break;
        case MENGER: renderMenger(image, width, height, realMin, realMax, imagMin, imagMax); break;
        case DRAGON: renderDragon(image, width, height, realMin, realMax, imagMin, imagMax); break;
        default: renderMandelbrot(image, width, height, realMin, realMax, imagMin, imagMax, maxIter); break;
    }
}

// Forward declarations for settings helpers (defined later in this file)
bool loadSettings(const std::string &path, double &centerReal, double &centerImag, double &width, int &fractal);
bool saveSettings(const std::string &path, double centerReal, double centerImag, double width, int fractal);

int main() {
    const int WIDTH = 640;
    const int HEIGHT = 480;
    const int MAX_ITER = 100;
    
    // Create window
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot Fractal");
    window.setFramerateLimit(60);
    
    // Create image to draw fractal
    sf::Image image;
    image.create(WIDTH, HEIGHT);
    
    // Create texture and sprite to display image
    sf::Texture texture;
    sf::Sprite sprite;
    // Prepare GPU texture once to enable fast updates and avoid recreating texture objects
    texture.create(WIDTH, HEIGHT);
    sprite.setTexture(texture);
    
    // Calculate fractal
    // Initial view bounds (will be updated by input or loaded from settings)
    const double INIT_REAL_MIN = -2.5;
    const double INIT_REAL_MAX = 1.0;
    const double INIT_IMAG_MIN = -1.0;
    const double INIT_IMAG_MAX = 1.0;

    const std::string settingsPath = "C:/_AI/002/fractal_settings.txt"; // stored in C:/_AI/002

    double realMin, realMax, imagMin, imagMax;
    int currentFractal = MADELBROT;

    // Try to load persisted center & width & fractal; if present, derive bounds preserving aspect ratio
    double savedCenterReal = 0.0, savedCenterImag = 0.0, savedWidth = 0.0;
    int savedFractal = MADELBROT;
    bool haveSaved = loadSettings(settingsPath, savedCenterReal, savedCenterImag, savedWidth, savedFractal);
    currentFractal = savedFractal;

    if (haveSaved && savedWidth > 0.0) {
        double halfW = savedWidth / 2.0;
        double heightSpan = savedWidth * (static_cast<double>(HEIGHT) / static_cast<double>(WIDTH));
        double halfH = heightSpan / 2.0;
        realMin = savedCenterReal - halfW;
        realMax = savedCenterReal + halfW;
        imagMin = savedCenterImag - halfH;
        imagMax = savedCenterImag + halfH;
    } else {
        realMin = INIT_REAL_MIN;
        realMax = INIT_REAL_MAX;
        imagMin = INIT_IMAG_MIN;
        imagMax = INIT_IMAG_MAX;
        // save initial state for next run
        double initCenterReal = (realMin + realMax) / 2.0;
        double initCenterImag = (imagMin + imagMax) / 2.0;
        double initWidth = realMax - realMin;
        saveSettings(settingsPath, initCenterReal, initCenterImag, initWidth, currentFractal);
    }

    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
    // Push pixels to the GPU texture (fast update) and draw via sprite to avoid flicker
    texture.update(image);

    // Debounced settings save: mark view dirty on changes and write after inactivity
    bool viewDirty = false;
    sf::Clock saveClock;
    const float saveDebounceSec = 0.2f; // 200 ms

    // Setup overlay text (requires a font); try Windows Arial first
    sf::Font font;
    bool fontLoaded = false;
    if (font.loadFromFile("C:/Windows/Fonts/arial.ttf")) fontLoaded = true;
    else if (font.loadFromFile("arial.ttf")) fontLoaded = true; // fallback if user provides font

    sf::Text overlay;
    if (fontLoaded) {
        overlay.setFont(font);
        overlay.setCharacterSize(14);
        overlay.setFillColor(sf::Color::White);
        overlay.setOutlineColor(sf::Color::Black);
        overlay.setOutlineThickness(1.0f);
        overlay.setPosition(8.f, 8.f);
    }
    
    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                    // save pending changes before exit
                    if (viewDirty) {
                        double centerReal = (realMin + realMax) / 2.0;
                        double centerImag = (imagMin + imagMax) / 2.0;
                        double width = (realMax - realMin);
                        saveSettings(settingsPath, centerReal, centerImag, width, currentFractal);
                        viewDirty = false;
                    }
                    window.close();
                }
            // Mouse left click: recenter the view on the clicked complex coordinate
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int mx = event.mouseButton.x;
                int my = event.mouseButton.y;

                double clickedReal = pixelToReal(mx, WIDTH, realMin, realMax);
                // Pixel Y=0 is the top of the window; map that to imagMax (top of complex plane)
                double clickedImag = pixelToImag(my, HEIGHT, imagMin, imagMax);

                double halfW = (realMax - realMin) / 2.0;
                double halfH = (imagMax - imagMin) / 2.0;

                realMin = clickedReal - halfW;
                realMax = clickedReal + halfW;
                imagMin = clickedImag - halfH;
                imagMax = clickedImag + halfH;

                // Diagnostic: log the click mapping and resulting bounds so we can verify correctness
                std::cout << "MouseClick px=(" << mx << "," << my << ") -> complex=(" << clickedReal << "," << clickedImag << ")\n";
                std::cout << "New bounds after click: [" << realMin << "," << realMax << "," << imagMin << "," << imagMax << "]\n";

                // Persist click info to a simple log for detached runs
                try {
                    std::ofstream clog("C:/_AI/002/clicks.log", std::ios::app);
                    if (clog.is_open()) {
                        clog << "MouseClick px=(" << mx << "," << my << ") -> complex=(" << clickedReal << "," << clickedImag << ") ";
                        clog << "bounds=[" << realMin << "," << realMax << "," << imagMin << "," << imagMax << "]\n";
                        clog.close();
                    }
                } catch (...) {
                    // ignore file errors in diagnostics
                }

                renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                texture.update(image);
                // mark dirty and debounce save
                viewDirty = true;
                saveClock.restart();
            }
            // Mouse wheel scroll: zoom centered at mouse cursor
            else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    float delta = event.mouseWheelScroll.delta; // positive = up (zoom in)
                    int mx = static_cast<int>(event.mouseWheelScroll.x);
                    int my = static_cast<int>(event.mouseWheelScroll.y);

                    // Compute zoom factor from delta; using a smooth exponential scale
                    double zoomFactor = std::pow(0.98, delta);

                    // Map the pixel under the cursor to complex coordinates
                    double clickR = pixelToReal(mx, WIDTH, realMin, realMax);
                    double clickI = pixelToImag(my, HEIGHT, imagMin, imagMax);

                    // Scale bounds around the clicked complex point
                    realMin = clickR + (realMin - clickR) * zoomFactor;
                    realMax = clickR + (realMax - clickR) * zoomFactor;
                    imagMin = clickI + (imagMin - clickI) * zoomFactor;
                    imagMax = clickI + (imagMax - clickI) * zoomFactor;

                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);

                    // mark dirty for debounced save
                    viewDirty = true; saveClock.restart();
                }
            }
            // Key pressed/released: enable continuous zoom while key is held
            else if (event.type == sf::Event::KeyPressed) {
                // Numpad Add/Subtract or regular keys (Equal with Shift for '+', Hyphen/Dash for '-')
                if (event.key.code == sf::Keyboard::Add || (event.key.code == sf::Keyboard::Equal && event.key.shift)) {
                    // start zoom-in (flags handled by polling below)
                } else if (event.key.code == sf::Keyboard::R) {
                    // Reset view to initial bounds
                    realMin = -2.5;
                    realMax = 1.0;
                    imagMin = -1.0;
                    imagMax = 1.0;

                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    // mark dirty for reset state and debounce save
                    {
                        viewDirty = true;
                        saveClock.restart();
                    }
                } else if (event.key.code == sf::Keyboard::Num1) {
                    currentFractal = MADELBROT;
                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    // mark dirty and debounce save for fractal change
                    viewDirty = true; saveClock.restart();
                    // debug: save current rendered image for inspection
                    image.saveToFile("C:/_AI/002/debug_fractal_1.png");
                } else if (event.key.code == sf::Keyboard::Num2) {
                    currentFractal = SIERPINSKI;
                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    viewDirty = true; saveClock.restart();
                    image.saveToFile("C:/_AI/002/debug_fractal_2.png");
                } else if (event.key.code == sf::Keyboard::Num3) {
                    currentFractal = KOCH;
                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    viewDirty = true; saveClock.restart();
                    image.saveToFile("C:/_AI/002/debug_fractal_3.png");
                } else if (event.key.code == sf::Keyboard::Num4) {
                    currentFractal = MENGER;
                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    viewDirty = true; saveClock.restart();
                    image.saveToFile("C:/_AI/002/debug_fractal_4.png");
                } else if (event.key.code == sf::Keyboard::Num5) {
                    currentFractal = DRAGON;
                    renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
                    texture.update(image);
                    viewDirty = true; saveClock.restart();
                    image.saveToFile("C:/_AI/002/debug_fractal_5.png");
                } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Hyphen) {
                    // start zoom-out
                }
            } else if (event.type == sf::Event::KeyReleased) {
                // nothing special handled here; continuous flags handled below via polling
            }
        }
        // Poll keyboard state for continuous zooming (support holding '+' or '-' keys)
        static bool zoomInPressed = false;
        static bool zoomOutPressed = false;
        // update flags using real-time keyboard state so both left/right and text variants work
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Add) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Equal) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))) {
            zoomInPressed = true;
        } else {
            // if Add not pressed and Equal+Shift not pressed, clear zoomIn
            zoomInPressed = false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract) || sf::Keyboard::isKeyPressed(sf::Keyboard::Hyphen)) {
            zoomOutPressed = true;
        } else {
            zoomOutPressed = false;
        }

        // continuous zoom timer: apply zoom every 50ms while key is held
        static sf::Clock zoomClock;
        const float zoomIntervalSec = 0.05f; // 50 ms
            if ((zoomInPressed || zoomOutPressed) && zoomClock.getElapsedTime().asSeconds() >= zoomIntervalSec) {
            double zoomFactor = 1.0;
            if (zoomInPressed && !zoomOutPressed) zoomFactor = 0.98; // zoom in 2%
            else if (zoomOutPressed && !zoomInPressed) zoomFactor = 1.02; // zoom out 2%

            // Center stays the same (current center)
            double centerReal = (realMin + realMax) / 2.0;
            double centerImag = (imagMin + imagMax) / 2.0;

            double halfWidth = (realMax - realMin) / 2.0 * zoomFactor;
            double halfHeight = (imagMax - imagMin) / 2.0 * zoomFactor;

            realMin = centerReal - halfWidth;
            realMax = centerReal + halfWidth;
            imagMin = centerImag - halfHeight;
            imagMax = centerImag + halfHeight;

            renderCurrent(currentFractal, image, WIDTH, HEIGHT, realMin, realMax, imagMin, imagMax, MAX_ITER);
            texture.update(image);

            // debounce zoomed state save
            viewDirty = true;
            saveClock.restart();

            zoomClock.restart();
        }

        // Handle smooth zooming via mouse wheel (centered on cursor)
        // Note: We handle MouseWheelScrolled events in the event loop; however
        // some platforms also provide continuous wheel info via real-time polling.

        // update overlay text with zoom and center and current fractal
        if (fontLoaded) {
            double centerReal = (realMin + realMax) / 2.0;
            double centerImag = (imagMin + imagMax) / 2.0;
            double currentWidth = (realMax - realMin);
            double initialWidth = (INIT_REAL_MAX - INIT_REAL_MIN);
            double zoomFactor = initialWidth / currentWidth; // 1.0 = initial

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6);
            oss << "Zoom: " << zoomFactor << "x (" << std::setprecision(2) << (zoomFactor * 100.0) << "%)\n";
            oss << std::setprecision(8);
            oss << "Center: (" << centerReal << ", " << centerImag << ")\n";
            const char *name = "MADELBROT";
            if (currentFractal == SIERPINSKI) name = "Sierpinski";
            else if (currentFractal == KOCH) name = "Koch";
            else if (currentFractal == MENGER) name = "Menger";
            else if (currentFractal == DRAGON) name = "Dragon";
            oss << "Fractal: " << name;
            overlay.setString(oss.str());
        }

        window.clear();
        window.draw(sprite);
        if (fontLoaded) window.draw(overlay);
        window.display();

        // Flush debounced save if enough idle time passed
        if (viewDirty && saveClock.getElapsedTime().asSeconds() >= saveDebounceSec) {
            double centerReal = (realMin + realMax) / 2.0;
            double centerImag = (imagMin + imagMax) / 2.0;
            double width = (realMax - realMin);
            saveSettings(settingsPath, centerReal, centerImag, width, currentFractal);
            viewDirty = false;
        }
    }
    
    return 0;
}

// Settings file helpers — store center and width so aspect ratio is preserved
bool loadSettings(const std::string &path, double &centerReal, double &centerImag, double &width, int &fractal) {
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                try {
                    if (key == "centerReal") centerReal = std::stod(value);
                    else if (key == "centerImag") centerImag = std::stod(value);
                    else if (key == "width") width = std::stod(value);
                    else if (key == "fractal") fractal = std::stoi(value);
                } catch (...) {
                    // ignore parse errors
                }
            }
        }
    }
    return true;
}

bool saveSettings(const std::string &path, double centerReal, double centerImag, double width, int fractal) {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;
    out << "centerReal=" << std::setprecision(15) << centerReal << "\n";
    out << "centerImag=" << std::setprecision(15) << centerImag << "\n";
    out << "width=" << std::setprecision(15) << width << "\n";
    out << "fractal=" << fractal << "\n";
    return true;
}
