#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define KOBO_MOUNT_POINT "/Volumes/KOBOeReader"
#define SQLITE_DB_PATH "/Volumes/KOBOeReader/.kobo/KoboReader.sqlite"
#define EXPORT_DIR "exported_annotations_MLA"

/* Function to create an export directory */
void createExportDirectory() {
    struct stat st = {0};

    if (stat(EXPORT_DIR, &st) == -1) {
        if (mkdir(EXPORT_DIR, 0755) == 0) {
            printf("Created export directory: %s\n", EXPORT_DIR);
        } else {
            perror("Failed to create export directory");
            exit(1);
        }
    }
}

/* Function to format date into MLA style (e.g., "21 Feb. 2024") */
void formatDateMLA(const char *inputDate, char *outputDate) {
    struct tm tm = {0};
    strptime(inputDate, "%Y-%m-%d %H:%M:%S", &tm);

    const char *months[] = {
        "Jan.", "Feb.", "Mar.", "Apr.", "May", "June",
        "July", "Aug.", "Sept.", "Oct.", "Nov.", "Dec."
    };

    snprintf(outputDate, 32, "%d %s %d", tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900);
}

/* Function to sanitize bookID */
void sanitizeBookID(char *bookID) {
    for (int i = 0; bookID[i]; i++) {
        if (bookID[i] == '/' || bookID[i] == '\\' || bookID[i] == ':' || bookID[i] == '\0') {
            bookID[i] = '_';
        }
    }
}

/* Function to get the author's last name and book title */
void getBookMetadata(sqlite3 *db, const char *volumeID, char *author, char *title) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT Title, Attribution FROM content WHERE ContentID = ? LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, volumeID, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            snprintf(title, 256, "%s", sqlite3_column_text(stmt, 0));
            snprintf(author, 256, "%s", sqlite3_column_text(stmt, 1));
        }
    }
    
    sqlite3_finalize(stmt);
}

/* Function to export annotations in MLA format */
void exportAnnotations() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open(SQLITE_DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "SELECT Text, VolumeID, Annotation, DateCreated FROM Bookmark "
                      "WHERE Text IS NOT NULL AND LENGTH(Text) > 0;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    createExportDirectory();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *annotation = (const char *)sqlite3_column_text(stmt, 0);
        const char *bookID = (const char *)sqlite3_column_text(stmt, 1);
        const char *userNote = (const char *)sqlite3_column_text(stmt, 2);
        const char *dateCreated = (const char *)sqlite3_column_text(stmt, 3);

        // Format the date in MLA style
        char formattedDate[32];
        formatDateMLA(dateCreated, formattedDate);

        // Retrieve book metadata
        char author[256] = "Unknown Author";
        char title[256] = "Unknown Title";
        getBookMetadata(db, bookID, author, title);

        // Sanitize bookID for filenames
        char sanitizedBookID[256];
        strncpy(sanitizedBookID, bookID, sizeof(sanitizedBookID) - 1);
        sanitizeBookID(sanitizedBookID);

        // Create filename
        char filename[512];
        snprintf(filename, sizeof(filename), "%s/%s.txt", EXPORT_DIR, sanitizedBookID);

        // Append annotation to the file
        FILE *file = fopen(filename, "a");
        if (file) {
            fprintf(file, "====================================\n");
            fprintf(file, "\"%s\" (%s, %s).\n", annotation, author, title);
            if (userNote && strlen(userNote) > 0) {
                fprintf(file, "[User Note] %s\n", userNote);
            }
            fprintf(file, "[Highlighted on: %s]\n", formattedDate);
            fprintf(file, "====================================\n\n");
            fclose(file);
            printf("Exported MLA citation to %s\n", filename);
        } else {
            perror("Error writing annotation file");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    DIR* dir = opendir(KOBO_MOUNT_POINT);
    if (!dir) {
        printf("Kobo eReader is not mounted at %s. Please check your device.\n", KOBO_MOUNT_POINT);
        return 1;
    }
    closedir(dir);

    printf("Kobo eReader detected! Extracting annotations in MLA format...\n");
    exportAnnotations();
    printf("Annotation export completed.\n");

    return 0;
}
