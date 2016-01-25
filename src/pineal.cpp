#include <SFML/Graphics.hpp>
#include <pineal.hpp>
#include <memory>
#include <iostream>
static const double pi = 3.141592654;

using namespace std;
//using namespace std::__cxx11;

#define check_attribute(__n, __name, __code) \
    if (s.n() == __n && !key.compare(__name)) { \
        __code; \
        return; \
    }


template<class T, typename K>
T* memorize(K name) {
    // I can not use templates in SWIG interfaces
    static map<K, unique_ptr<T>> memory;

    if (memory.count(name) == 0) {
        memory[name] = unique_ptr<T>(new T(name));
    }
    return memory[name].get();
}

// Entity
void Entity::attribute(string key, Signal s) {
    attributes[key] = s;
}

void Entity::apply_all_attributes() {
    for (const auto& p : attributes) {
        apply_attribute(p.first, p.second);
    }
}
//

// Group
void Group::add(Drawable* d) {
    elements.push_back(d);
}
Group::Group(string name) : Group() {
}

void Group::apply_attribute(string key, Signal s) {
    check_attribute(1, "translate",
                    sf_transform.translate(s.x(), 0));

    check_attribute(2, "translate",
                    sf_transform.translate(s.x(), s.y()));

    check_attribute(1, "rotate",
                    sf_transform.rotate(180 * s.x() / pi));

    check_attribute(1, "scale",
                    sf_transform.scale(s.x(), s.x()));

    check_attribute(2, "scale",
                    sf_transform.scale(s.x(), s.y()));

    for (Drawable *e : elements)
        e->apply_attribute(key, s);
}

void Group::draw(sf::RenderTarget* target, sf::RenderStates states) {
    sf_transform = sf::Transform();
    apply_all_attributes();

    states.transform *= sf_transform;

    for (Drawable *e : elements) {
        e->draw(target, states);
    }
}
//


// Polygon
Polygon::Polygon(int n) {
    double r = 1;
    sf_shape = sf::CircleShape(r);
    sf_shape.setPointCount(n);

    sf_shape.setOrigin(r, r);
}

void Polygon::apply_attribute(string key, Signal s) {
    Color c(s.x(), s.y(), s.z(), s.w());

    check_attribute(1, "line",
                    sf_shape.setOutlineThickness(s.x()));

    check_attribute(1, "rotation",
                    sf_shape.setRotation(180 * s.x()/pi));

    check_attribute(1, "radius",
                    sf_shape.setScale(s.x(), s.x()));

    check_attribute(2, "position",
                    sf_shape.setPosition(s.x(), s.y()));

    check_attribute(4, "fill",
                    sf_shape.setFillColor(sf::Color(c.r()*255,
                                                    c.g()*255,
                                                    c.b()*255,
                                                    c.a()*255)));

    check_attribute(4, "stroke",
                    sf_shape.setOutlineColor(sf::Color(c.r()*255,
                                                       c.g()*255,
                                                       c.b()*255,
                                                       c.a()*255)));
}

void Polygon::draw(sf::RenderTarget* target, sf::RenderStates states) {
    apply_all_attributes();
    target->draw(sf_shape, states);
}
//

// Layer
Layer::Layer() : w(2000), h(2000) {
    sf_shape = sf::ConvexShape(4);
    render_texture.create(w, h);

    sf_shape.setPoint(0, sf::Vector2f(-1, -1));
    sf_shape.setPoint(1, sf::Vector2f(1, -1));
    sf_shape.setPoint(2, sf::Vector2f(1, 1));
    sf_shape.setPoint(3, sf::Vector2f(-1, 1));
}

Layer::Layer(string name) : Layer() {
}

void Layer::render(Drawable* child) {
    sf::View view;
    view.setCenter(0, 0);
    view.setSize(2, 2);

    render_texture.setView(view);
    render_texture.clear();
    render_texture.setSmooth(true);
    child->draw(&render_texture, sf::RenderStates());
    render_texture.display();
}

void Layer::apply_attribute(string key, Signal s) {
    check_attribute(2, "p0",
                    sf_shape.setPoint(0, sf::Vector2f(s.x(), s.y())));

    check_attribute(2, "p1",
                    sf_shape.setPoint(1, sf::Vector2f(s.x(), s.y())));

    check_attribute(2, "p2",
                    sf_shape.setPoint(2, sf::Vector2f(s.x(), s.y())));

    check_attribute(2, "p3",
                    sf_shape.setPoint(3, sf::Vector2f(s.x(), s.y())));
}


void Layer::draw(sf::RenderTarget* target, sf::RenderStates states) {
    apply_all_attributes();

    const sf::Texture& texture = render_texture.getTexture();
    sf::Sprite sprite(texture);

    sf_shape.setTexture(&texture);
    target->draw(sf_shape, states);
}

Layer* Layer::memo(string name) {
    return memorize<Layer>(name);
}
//

// Window
Window::Window(string name) {
    render_window.create(sf::VideoMode(800, 600), name);
}

bool Window::is_open() {
    return render_window.isOpen();
}

void Window::render(Drawable* child) {
    sf::Event event;

    while (render_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            render_window.close();
    }

    sf::View view;
    sf::Vector2<unsigned int> size = render_window.getSize();
    double w = size.x,
           h = size.y;
    view.setCenter(0, 0);
    view.setSize(2, 2 * h/w);

    render_window.clear(sf::Color::Black);
    render_window.setView(view);

    child->draw(&render_window, sf::RenderStates());
    render_window.display();
}

Window* Window::memo(string name) {
    return memorize<Window>(name);
}
//