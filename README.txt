This project uses a future-proof code style, anticipating future features coming to Epsilon. For example:

This pattern might looks silly:

    F {
        A?:a,
        B?:b,
        C?:c
    }

It however allows code to be easily adjusted once Epsilon supports polymorphism. The above snippet could be converted into:

    @super
    F {}

    @extends F
    A {}

    @extends F
    B {}

    @extends F
    C {}

Where occurrences of `F` are converted to `:F`.
