# SCTP_Test

## Purpose

Generate certain chunks introduced in later standards

## Usage

1. Build binaries: `$ make`
2. Create namespaces: `# bash ./create-ns.sh`
3. [Start `tcpdump` on one of the namespace]: `# ip netns exec client tcpdump not arp -vv`
4. Run tests
  1. Start server: `# ip netns exec server ./sctp_server`
  2. Start test cases:
    - `$ ip netns exec client ./sctp_client_*`
