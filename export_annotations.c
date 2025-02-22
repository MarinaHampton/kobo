#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define KOBO_MOUNT_POINT "/Volumes/KOBOeReader"
#define SQLITE_DB_PATH "/Volumes/KOBOeReader/.kobo/KoboReader.sqlite"
#define EXPORT_DIR "exported_annotations"

/* Function to create an export directory */
void createExportDirectory() {
    struct stat st = {0};
    
    // Check if directory exists, if not, create it
    if (stat(EXPORT_DIR, &st) == -1) {
        if (mkdir(EXPORT_DIR, 0755) == 0) {
            printf("Created export directory: %s\n", EXPORT_DIR);
        } else {
            perror("Failed to create export directory");
            exit(1);
        }
    }
}

/* Function to sanitize bookID (removing characters that might cause filename issues) */
void sanitizeBookID(char *bookID) {
    for (int i = 0; bookID[i]; i++) {
        if (bookID[i] == '/' || bookID[i] == '\\' || bookID[i] == ':' || bookID[i] == '\0') {
            bookID[i] = '_'; // Replace invalid characters with underscores
        }
    }
}

/* Function to extract annotations from the Kobo SQLite database */
void exportAnnotations() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // Open Kobo's SQLite database
    rc = sqlite3_open(SQLITE_DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // SQL query to fetch annotations
    const char *sql = "SELECT Text, VolumeID FROM Bookmark WHERE Text IS NOT NULL AND LENGTH(Text) > 0;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    createExportDirectory();

    // Process each row
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *annotation = (const char *)sqlite3_column_text(stmt, 0);
        const char *bookID = (const char *)sqlite3_column_text(stmt, 1);

        // Sanitize bookID to prevent file system issues
        char sanitizedBookID[256];
        strncpy(sanitizedBookID, bookID, sizeof(sanitizedBookID) - 1);
        sanitizeBookID(sanitizedBookID);

        // Create a file per book
        char filename[512];
        snprintf(filename, sizeof(filename), "%s/%s.txt", EXPORT_DIR, sanitizedBookID);

        // Print debug output for the filename
        printf("Attempting to write annotation to file: %s\n", filename);

        // Append annotation to the file
        FILE *file = fopen(filename, "a");
        if (file) {
            fprintf(file, "%s\n", annotation);
            fclose(file);
            printf("Exported annotation to %s\n", filename);
        } else {
            perror("Error writing annotation file");
        }
    }

    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    // Check if the Kobo eReader is mounted
    DIR* dir = opendir(KOBO_MOUNT_POINT);
    if (!dir) {
        printf("Kobo eReader is not mounted at %s. Please check your device.\n", KOBO_MOUNT_POINT);
        return 1;
    }
    closedir(dir);

    printf("Kobo eReader detected! Extracting annotations...\n");
    exportAnnotations();
    printf("Annotation export completed.\n");

    return 0;
}
