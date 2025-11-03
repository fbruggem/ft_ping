
# Use Debian 13 (codename "trixie") base
FROM debian:13

# Install dependencies for Nix
RUN apt-get update && apt-get install -y \
    curl \
    xz-utils \
    sudo \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create a non‑root user (optional, recommended)
ARG USERNAME=devuser
RUN useradd -m -s /bin/bash ${USERNAME} && \
    echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME}

# Install Nix (multi‑user daemon mode)
RUN sh <(curl -L https://nixos.org/nix/install) --daemon

# Switch to the non‑root user
USER ${USERNAME}
WORKDIR /home/${USERNAME}

# Source Nix profile (so Nix commands work)
RUN echo ". /home/${USERNAME}/.nix-profile/etc/profile.d/nix.sh" >> /home/${USERNAME}/.bashrc

# Copy your current nix‑shell environment setup (if you have one) into the container
# For example, if you have a shell.nix in your project, copy it:
COPY shell.nix /home/${USERNAME}/project/shell.nix
WORKDIR /home/${USERNAME}/project

# Make sure bash login picks up the shell environment: run nix-shell on login
RUN echo 'exec nix-shell shell.nix' >> /home/${USERNAME}/.bashrc

CMD [ "bash", "--login" ]
