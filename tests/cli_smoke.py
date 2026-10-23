"""Run the actual console app against isolated data, never the user's data/."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def run(executable, directory, lines=(), *arguments):
    result = subprocess.run(
        [str(executable), "--data-dir", str(directory), *arguments],
        input="\n".join(lines) + "\n",
        capture_output=True,
        encoding="utf-8",
        timeout=30,
    )
    if result.returncode:
        raise AssertionError(result.stdout + result.stderr)
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=Path)
    executable = parser.parse_args().executable.resolve()
    with tempfile.TemporaryDirectory(prefix="student-management-cli-") as temporary:
        directory = Path(temporary) / "data"
        output = run(executable, directory, [
            # Bootstrap and reject malformed menu input / incorrect credentials.
            "admin", "admin-pass", "invalid", "9", "1", "admin", "wrong",
            "1", "admin", "admin-pass",
            # Teacher, two students, subject and a one-seat section.
            "2", "2", "T001", "Test Teacher", "1", "1", "1980", "teacher@example.test", "0",
            "1", "2", "S001", "Nguy\u1ec5n V\u0103n An", "1", "1", "2005", "one@example.test",
            "2", "S002", "Student Two", "1", "1", "2005", "two@example.test", "0",
            "3", "2", "M001", "C++", "3", "0",
            "4", "2", "C001", "M001", "T001", "1", "0",
            # Explicit account-to-profile mapping.
            "6", "2", "teacher", "teacher-pass", "1", "T001",
            "2", "student", "student-pass", "0", "S001",
            "2", "student2", "student-pass", "0", "S002", "0",
            # Register first student; a second registration must fail on capacity.
            "5", "2", "E001", "S001", "C001", "23", "10", "2026",
            "2", "E002", "S002", "C001", "23", "10", "2026", "5", "E001", "0", "0",
            # Teacher grades, views the section, then logs out.
            "1", "teacher", "teacher-pass", "3", "E001", "10", "8", "7", "2", "C001", "0",
            # Student checks own results and changes password.
            "1", "student", "student-pass", "2", "3", "student-pass", "new-pass", "new-pass", "0", "0",
        ])
        for expected in ["Hay nhap mot so hop le", "Lua chon phai tu 0 den 1", "Sai ten dang nhap",
                         "Lop da day", "7.60", "Da luu du lieu"]:
            if expected not in output:
                raise AssertionError(f"Missing expected output: {expected}\n{output}")
        if "Nguy\u1ec5n V\u0103n An" not in (directory / "students.txt").read_text(encoding="utf-8"):
            raise AssertionError("Vietnamese UTF-8 input was not saved correctly")

        checked = run(executable, directory, (), "--check-data")
        if "Du lieu hop le" not in checked:
            raise AssertionError(checked)
        restarted = run(executable, directory, ["1", "student", "new-pass", "2", "0", "0"])
        if "E001" not in restarted or "7.60" not in restarted or "S002" in restarted:
            raise AssertionError("Restart / student visibility failed:\n" + restarted)
        other_student = run(executable, directory, ["1", "student2", "student-pass", "2", "0", "0"])
        if "Chua co dang ky hoc" not in other_student or "E001" in other_student:
            raise AssertionError("Other student can see private grades:\n" + other_student)
        # EOF halfway through a form exits without inserting a partial record.
        run(executable, directory, ["1", "admin", "admin-pass", "1", "2", "PARTIAL"])
        if "PARTIAL" in (directory / "students.txt").read_text(encoding="utf-8"):
            raise AssertionError("Incomplete form created a record")
    print("CLI smoke passed: bootstrap, CRUD, enrollment, capacity, roles, grades, password, restart, EOF.")


if __name__ == "__main__":
    main()
