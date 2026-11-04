"""Portable Windows entry point; Python is embedded in the released Setup.exe."""
import sys


def main():
    for stream in (sys.stdout, sys.stderr):
        if stream is not None and hasattr(stream, 'reconfigure'):
            stream.reconfigure(encoding='utf-8', errors='replace')
    if len(sys.argv) > 1 and sys.argv[1] == '--ending-worker':
        from prepare_ending import ending_worker_main
        ending_worker_main(sys.argv[2])
        return 0
    if len(sys.argv) > 1 and sys.argv[1] == '--prepare-worker':
        # The same frozen program is the child, not an external Python install.
        sys.argv = [sys.argv[0], *sys.argv[2:]]
        from prepare_assets import main as prepare_main
        prepare_main()
        return 0
    interactive = len(sys.argv) == 1
    from setup_assets import main as setup_main
    code = setup_main()
    if interactive:
        try:
            input('\nPress Enter to close this window...')
        except (EOFError, KeyboardInterrupt):
            pass
    return code


if __name__ == '__main__':
    raise SystemExit(main())
