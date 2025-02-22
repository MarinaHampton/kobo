**📄 Kobo Annotation Exporter**
Overview
This tool extracts annotations (highlights and notes) from a Kobo eReader's SQLite database and saves them into text files, formatted in MLA citation style.

**Requirements**
A Kobo eReader, mounted at /Volumes/KOBOeReader
A Mac or Linux system with:
gcc (for compiling)
sqlite3 library installed

**Installation & Setup**
1️⃣ Clone or Download the Project
Open Terminal and navigate to your desired directory:

sh
Copy
Edit
cd ~/Desktop
git clone https://github.com/your-repo/kobo_features.git
cd kobo_features
(If you don’t have git, just download the files manually.)

2️⃣ Ensure Your Kobo is Mounted
Before running the program, confirm your Kobo is properly connected. Run:

sh
Copy
Edit
ls /Volumes
You should see KOBOeReader listed.

3️⃣ Compile the Program
Run:

sh
Copy
Edit
gcc -o export_MLA export_MLA.c -lsqlite3
This creates the export_MLA executable.

4️⃣ Run the Export Tool
Execute the program:

sh
Copy
Edit
./export_MLA
If everything is correct, you’ll see output confirming that annotations are being extracted.

How It Works
The program connects to KoboReader.sqlite on the mounted Kobo.
It extracts highlighted text and notes from the Bookmark table.
The output is saved in an exported_annotations/ folder.
Each book gets a separate text file (BookID.txt) containing annotations in MLA format.
Troubleshooting
🔹 Kobo Not Detected?
Run ls /Volumes again.
Try unplugging and reconnecting your Kobo.
Make sure the eReader is in USB mode.
🔹 Compilation Issues?
If you get an error about missing SQLite, install the library:

sh
Copy
Edit
brew install sqlite
or on Linux:

sh
Copy
Edit
sudo apt install libsqlite3-dev
🔹 SQLite File Not Found?
Run:

sh
Copy
Edit
ls -l /Volumes/KOBOeReader/.kobo/KoboReader.sqlite
If missing, ensure your Kobo is properly connected.

Future Improvements
Export annotations as Markdown or JSON
GUI version for easier use
✅ Notes
Make sure your Kobo is unlocked before connecting.
All exported annotations are saved in exported_annotations/.
