{
    "targets": [
        {
            "target_name": "native",
            "cflags_cc": [
                "-std=c++17"
            ],
            "sources": [
                "src/cpp/main.cpp"
            ],
            "include_dirs": [
                "src/cpp/include"
                # "<!@(node -p \"require('napixx').include\")"
            ]
        }
    ]
}