float get_scaled_number( char *mark, float figure)

{
    int off;
    char *scaled_unit = "bkmgt";
    float scaled_figure = figure;

    for( off = 0; *(scaled_unit + off) && scaled_figure >= 1000.; )
    {
        scaled_figure /= 1024.0;
        off++;
    }

    if( mark) *mark = *(scaled_unit + off);

    return( scaled_figure);
}
