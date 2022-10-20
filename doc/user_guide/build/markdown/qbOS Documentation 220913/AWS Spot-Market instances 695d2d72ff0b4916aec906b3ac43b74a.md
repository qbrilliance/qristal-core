# AWS Spot-Market instances

## VM through AWS

**Automated script setup**

<aside>
ℹ️ **Quantum Brilliance will contact you to issue this script**.

</aside>

The setup for qbOS is automated via a `cloud-init` script suitable for **Ubuntu 20.04 LTS (64-bit x86)**.  Once you have access to this script, proceed to use the AWS Console to create a VM running Ubuntu 20.04 LTS.  

Complete each required step until you reach the **3. Configure Instance** section:

![Untitled](AWS%20Spot-Market%20instances%20695d2d72ff0b4916aec906b3ac43b74a/Untitled.png)

Then, under **User data,** paste (as text) the `cloud-init` script that has been issued to you by Quantum Brilliance.

Now complete all remaining steps as you prefer and launch your instance.