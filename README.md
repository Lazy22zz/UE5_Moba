UE5-MOBA MULTIPLAYER ONLINE 

 # 1, Basic Character Movement and Animation
 
 - 1, Set up Base Class
     1. Create basic classes: gamemode, playercharacter, playercontroller
     2. Assign the new gamemode to the project and modify the gamemode.

- 2, Set up the Camera and spring component
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/bb79df51a122d6f3992b6581913d85003c9085f0)
 
- 3, Add the jump Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/3587d6761cc81668069bce302bfd820d1f5ba688)
     2. Assign the input action and input mapping to the bp_playercharacter.

- 4, Add the look Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/4bd94ba7b4fdfbb07e7d70b58e38ac3c6b9945f0)
     2. Assign the IA_Look in BP_Character.
  
- 5, Add the Move Input Action
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/74f5d7e9db7afa5d82c2f775f2cb485873664e46)
     2. <img width="1055" height="678" alt="Screenshot 2025-09-10 192605" src="https://github.com/user-attachments/assets/68149941-20c7-4a3b-a05a-5eddc639abf3" />

- 6, Create the Animation Instance
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/e44113499653c8782cea020a6fa410d1bd222252)
     2. Create a new Animation Instance.
     3. <img width="532" height="74" alt="Screenshot 2025-09-11 201706" src="https://github.com/user-attachments/assets/f9d2031f-b9c2-4f64-843f-321f2e89c7da" />

- 7, Add Speed Attribute to Drive Ground Locomotion
     1. [code view](https://github.com/Lazy22zz/UE5_Moba/commit/00f1589532965f06edd250c280acdd0421c2349a)
     2. Set an automatic rule. <img width="525" height="34" alt="Screenshot 2025-09-12 202156" src="https://github.com/user-attachments/assets/dc63bca5-9814-472b-875c-341a772a87f3" />
     3. Using Group to smooth animations. <img width="519" height="108" alt="Screenshot 2025-09-12 202319" src="https://github.com/user-attachments/assets/3cbf344d-fcb1-43e6-a7bf-b229bc915385" />

