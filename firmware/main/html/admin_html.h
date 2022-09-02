
#pragma once
#include <string>
std::string admin_html = "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\"><meta name=\"author\" content=\"Jonathan Sönnerup and Christoffer Cederberg\"><title>Traffic Light</title><link href=\"https://getbootstrap.com/docs/4.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\"><style>.bd-placeholder-img {font-size: 1.125rem;text-anchor: middle;-webkit-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;}@media (min-width: 768px) {.bd-placeholder-img-lg {font-size: 3.5rem;}}hr {margin-top: 1rem;margin-bottom: 1rem;border: 0;border-top: 1px solid rgba(255, 255, 255, 0.1);}</style><link href=\"https://getbootstrap.com/docs/4.3/examples/cover/cover.css\" rel=\"stylesheet\"></head><body class=\"text-center\"><div class=\"cover-container d-flex w-100 h-100 p-3 mx-auto flex-column\"><header class=\"masthead mb-auto\"><div class=\"inner\"><h3 class=\"masthead-brand\">Traffic Lights - Admin</h3><nav class=\"nav nav-masthead justify-content-center\"><a class=\"nav-link active\" href=\"/\">Home</a><a class=\"nav-link\" href=\"/about\">About</a></nav></div></header><main role=\"main\" class=\"inner cover\"><h1 class=\"cover-heading\">%s</h1><form method=\"GET\"><div class=\"form-group\"><label for=\"text\">Traffic Light text:</label><input type=\"text\" class=\"form-control\" id=\"text\" placeholder=\"Skriv text här\" name=\"text\"></div><button type=\"submit\" class=\"btn btn-primary\">Update</button></form><br/><hr/><form method=\"GET\"><div class=\"form-group text-center d-flex justify-content-center align-items-center flex-column\"><input type=\"color\" id=\"stop\" name=\"stop\" value=\"#e66465\"><label for=\"head\">Stop color</label><input type=\"color\" id=\"go\" name=\"go\" value=\"#f6b73c\"><label for=\"body\">Go color</label></div><button type=\"submit\" class=\"btn btn-primary\">Update colors</button></form></main><footer class=\"mastfoot mt-auto\"><div class=\"inner\"><p>by Jonathan Sönnerup && Christoffer Cederberg.</p></div></footer></div></body></html>";
