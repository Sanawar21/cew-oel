#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>

typedef struct
{
  char *data;
  size_t length;
} Response;

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t total_size = size * nmemb;
  Response *response = (Response *)userdata;

  // Reallocate memory to hold the current data plus new data
  char *temp = realloc(response->data, response->length + total_size + 1);
  if (temp == NULL)
  {
    fprintf(stderr, "Failed to allocate memory\n");
    return 0; // Return 0 to indicate a failure to `curl`
  }

  response->data = temp;
  memcpy(response->data + response->length, ptr, total_size);
  response->length += total_size;
  response->data[response->length] = '\0'; // Null-terminate the string

  return total_size;
}

void render_html_without_head(const char *html)
{
  bool in_tag = false;
  bool in_head = false;

  for (const char *ptr = html; *ptr; ++ptr)
  {
    if (strncmp(ptr, "<head>", 6) == 0)
    {
      in_head = true;
      ptr += 5; // Move pointer to the end of "<head>"
    }
    else if (strncmp(ptr, "</head>", 7) == 0)
    {
      in_head = false;
      ptr += 6; // Move pointer to the end of "</head>"
    }
    else if (*ptr == '<')
    {
      in_tag = true;
    }
    else if (*ptr == '>')
    {
      in_tag = false;
    }
    else if (!in_tag && !in_head)
    {
      putchar(*ptr); // Print content outside tags and <head>
    }
  }
  putchar('\n');
}

int main()
{
  CURL *curl;
  CURLcode res;
  Response response;

  response.data = malloc(1); // Start with 1 byte for the string
  response.length = 0;
  response.data[0] = '\0'; // Initialize it as an empty string

  // Initialize a libcurl session
  curl = curl_easy_init();
  if (curl)
  {
    // Set URL for the request
    curl_easy_setopt(curl, CURLOPT_URL, "https://wttr.in/karachi");

    // Set callback function to handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
      render_html_without_head(response.data);
    }

    // Clean up the session
    curl_easy_cleanup(curl);
    free(response.data); // Free the allocated memory
  }

  return 0;
}
