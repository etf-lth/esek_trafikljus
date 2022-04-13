import os
import sys


template = """
#pragma once
#include <string>
std::string {0}_html = "{1}";
"""


def minimize(filename):
    with open(filename, 'r') as fp:
        lines = fp.read().splitlines()

    html = ""
    for line in lines:
        if line != "":  
            html += line.strip()

    # escape special chars
    html = html.replace('"', '\\"')
    return html


def save_html(filename, html):
    # get the filename, no extension
    cf = filename.split(".")
    # handle the two cases: "index.html", and "./index.html"
    c_filename = cf[0] if len(cf) == 2 else cf[1]

    root = os.environ['TRAFIKLJUS_ROOT']
    path = os.path.join(root, 'main', 'html', f'{c_filename}_html.h')

    with open(path, 'w') as fp:
        fp.write(template.format(c_filename, min_html))

    print('Wrote content to {}'.format(path))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <html file>")
        sys.exit(1)
    
    filename = sys.argv[1]
    min_html = minimize(filename)
    save_html(filename, min_html)