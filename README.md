VLLA-JS
=======

vlla-js __js file__

EXAMPLE
-------

    for(var y=0; y < 32; y++) {
        for(var x=0; x < 60; x++) {
            var c = (x ^ y) * 4 & 0xFF;
            palette(c, c, c);
            paint(x, y);
        }
    }

    refresh();


JS API
------

    palette(r, g, b);   // change the color of the paint
    paint(x, y);        // change the color of a pixel at (x, y)
    refresh();          // make changes visible on the display

    t();                // 32 bit integer that increases every frame
