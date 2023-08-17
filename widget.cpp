#include "widget.h"
#include "gadent.h"

struct Widget::Impl {
    Gadent g;
};

// Widget::Widget() : pImpl(new Impl) {
    
// }

Widget::Widget() : pImpl(std::make_unique<Impl>()) {}

Widget::~Widget() = default;

Widget::Widget(Widget&& rhs) = default;

Widget& Widget::operator=(Widget&& rhs) = default;