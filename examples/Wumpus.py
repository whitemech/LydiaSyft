import os

def generate_final_specs_and_partition():
    
    root_path = "" 
    
    rows, cols = 4, 4
    gold_positions = [(0, 1), (1, 2), (3, 2)]
    wumpus_start = (2, 3)
    agent_start = (3, 0)
    destination = (0, 0)
    
    actions = ["nord", "sud", "est", "ovest", "wait"]
    dest_str = f"at{destination[0]}{destination[1]}"
    
   
    output_dir = os.path.join(root_path, "wumpus_game")
    os.makedirs(output_dir, exist_ok=True)
    
    # # 1. FILE GENERATION: agent0.ltlf
    
    # # Sub-specification: Fcollect
    fcollect_parts = [f"F(at{r}{c})" for r, c in gold_positions]
    fcollect = " && ".join(fcollect_parts)
    
    # Sub-specification: Ftask
    ftask_pure = f"E(F(({fcollect}) && X[!](F({dest_str} && X(G(wait))))))"
    
    # Sub-specification: Fmutex
    fmutex_parts = []
    for i, act in enumerate(actions):
        other_actions = [f"!({other})" for j, other in enumerate(actions) if i != j]
        others_combined = " && ".join(other_actions)
        fmutex_parts.append(f"({act} -> ({others_combined}))")
    fmutex_pure = "A(G(" + " && ".join(fmutex_parts) + "))"
    
    # # Sub-specification: Favoid_wumpus
    avoid_rows_list = []
    for r in range(rows):
        row_cells = []
        for c in range(cols):
            row_cells.append(f"(at{r}{c} && wat{r}{c})")
        inner_row_or = " || ".join(row_cells)
        avoid_rows_list.append(f"A(G(!({inner_row_or})))")
    
    favoid_pure = " && ".join(avoid_rows_list)


    
    # Sub-specification: Fpre
    fpre_parts = []
    for r in range(rows):
        for c in range(cols):
            if r == 0:        fpre_parts.append(f"(at{r}{c} -> !nord)")
            if r == rows - 1: fpre_parts.append(f"(at{r}{c} -> !sud)")
            if c == 0:        fpre_parts.append(f"(at{r}{c} -> !ovest)")
            if c == cols - 1: fpre_parts.append(f"(at{r}{c} -> !est)")
    fpre_pure = "A(G(" + " && ".join(fpre_parts) + "))"
    
    # Final combined formula for Agent 0
    # {favoid_pure} && {ftask_pure} && {fmutex_pure} && {fpre_pure}
    f0_pure = f"{favoid_pure} && {ftask_pure} && {fmutex_pure} && {fpre_pure}"
    
    with open(os.path.join(output_dir, "agent0.ltlf"), "w") as f:
        f.write(f0_pure)
    print("File 'agent0.ltlf' successfully saved.")

    # 2. FILE GENERATION: agent1.ltlf
    
    wumpus_cells = [f"wat{r}{3}" for r in range(rows)]
    fwumpus_pure = "A(G(" + " || ".join(wumpus_cells) + "))"
    
    with open(os.path.join(output_dir, "agent1.ltlf"), "w") as f:
        f.write(fwumpus_pure)
    print("File 'agent1.ltlf' successfully saved.")

    # 3. FILE GENERATION: env.ltlf 
   
    einit_parts = []
    for r in range(rows):
        for c in range(cols):
            if (r, c) == agent_start:  einit_parts.append(f"at{r}{c}")
            else:                      einit_parts.append(f"!(at{r}{c})")
                
            if (r, c) == wumpus_start: einit_parts.append(f"wat{r}{c}")
            else:                      einit_parts.append(f"!(wat{r}{c})")
    einit_pure = " && ".join(einit_parts)
    
    etrans_list = []
    
    for r in range(rows):
        for c in range(cols):
            incoming = []
            if r + 1 < rows: incoming.append(f"(at{r+1}{c} && nord)")
            if r - 1 >= 0:   incoming.append(f"(at{r-1}{c} && sud)")
            if c - 1 >= 0:   incoming.append(f"(at{r}{c-1} && est)")
            if c + 1 < cols: incoming.append(f"(at{r}{c+1} && ovest)")
            
            incoming.append(f"(at{r}{c} && wait)")
            
            incoming = list(set(incoming))
            condition = " || ".join(incoming)
            
            etrans_list.append(f"X(G(X(at{r}{c} <-> ({condition}))))")
            
            
    for r in range(rows):
        wumpus_incoming = []
        if r + 1 < rows:  wumpus_incoming.append(f"wat{r+1}3")
        if r - 1 >= 0:    wumpus_incoming.append(f"wat{r-1}3")
        
        wumpus_condition = " || ".join(wumpus_incoming)
        
        etrans_list.append(f"X(G(X(wat{r}3 <-> ({wumpus_condition}))))")
        
        
    etrans_pure = " && ".join(etrans_list)
    # fenv_pure = f"({einit_pure}) && A({etrans_pure})"
    fenv_pure = f"A({etrans_pure})"

    with open(os.path.join(output_dir, "env.ltlf"), "w") as f:
        f.write(fenv_pure)
    print("File 'env.ltlf' successfully saved.")

    # # 4. FILE GENERATION: var.part
    
    part_lines = []
    part_lines.append(".inputs:")
    
    agent0_vars = [f"at{r}{c}" for r in range(rows) for c in range(cols)]
    agent0_vars.extend(actions)
    part_lines.append(".agent0:" + " ".join(agent0_vars))
    
    agent1_vars = [f"wat{r}{c}" for r in range(rows) for c in range(cols)]
    part_lines.append(".agent1:" + " ".join(agent1_vars))
    
    part_path = os.path.join(output_dir, "var.part")
    with open(part_path, "w") as f:
        f.write("\n".join(part_lines))
    print("File 'var.part' successfully saved.")
    

if __name__ == "__main__":
    generate_final_specs_and_partition()
