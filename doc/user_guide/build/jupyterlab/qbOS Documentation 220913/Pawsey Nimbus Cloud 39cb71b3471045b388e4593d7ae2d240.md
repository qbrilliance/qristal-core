# Pawsey Nimbus Cloud

## VM through Pawsey Nimbus Cloud

> The Pawsey Nimbus Research Cloud is exclusively for research purposes. It is free for anyone applying from an Australian research organisation. Western Australian State Government agencies undertaking research can also access resources free of charge.
> 
- **Prerequisites:**
    - **A prior allocation** on the Pawsey Nimbus Cloud has been awarded.  If you wish to apply for an allocation, visit this [link to the Pawsey Supercomputing Centre's application process](https://support.pawsey.org.au/documentation/display/US/Apply+for+Nimbus+Cloud+access).
    - If you need to familiarise yourself with cloud computing concepts, please see this [link](https://support.pawsey.org.au/documentation/display/US/Create+a+Nimbus+Instance) to Pawsey's documentation.

**It's also important to already be familiar with:**

- Starting and stopping VM instances from the Nimbus web interface: [https://nimbus.pawsey.org.au](https://nimbus.pawsey.org.au)
- Using SSH commands and SSH keys on your local computer to log into your VM instance on Nimbus.

**Automated script setup**

The setup for qbOS is automated via a `cloud-init` script suitable for **Ubuntu 18.04 LTS** -  Quantum Brilliance will contact you to issue this script.  Once you have access to this script, proceed to create a VM via the Nimbus web-interface.  Complete each required step until you reach the **Configuration** section.

Then, under **Customization Script,** paste the `cloud-init` script that has been issued to you by Quantum Brilliance:

![Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240/Untitled.png](Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240/Untitled.png)

Then click: **Launch Instance**